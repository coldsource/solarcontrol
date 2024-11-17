/*
 * This file is part of SolarControl
 *
 * SolarControl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SolarControl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SolarControl. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Thibault Kummer <bob@coldsource.net>
 */

#include <thread/DevicesManager.hpp>
#include <datetime/Timestamp.hpp>
#include <device/Devices.hpp>
#include <device/DevicesOnOff.hpp>
#include <device/DeviceOnOff.hpp>
#include <energy/GlobalMeter.hpp>
#include <websocket/SolarControl.hpp>
#include <configuration/ConfigurationSolarControl.hpp>
#include <logs/Logger.hpp>

#include <stdexcept>

using namespace std;
using namespace device;
using datetime::Timestamp;

namespace thread {

DevicesManager *DevicesManager::instance = 0;

DevicesManager::DevicesManager():
available_power_avg(configuration::ConfigurationSolarControl::GetInstance()->GetInt("control.hysteresis.smoothing") * 60)
{
	start();

	auto config = configuration::ConfigurationSolarControl::GetInstance();
	hysteresis_export = config->GetInt("control.hysteresis.export");
	hysteresis_import = config->GetInt("control.hysteresis.import");
	state_update_interval = config->GetInt("control.state.update_interval");
	cooldown = config->GetInt("control.cooldown");

	global_meter = energy::GlobalMeter::GetInstance();

	instance = this;
}

bool DevicesManager::hysteresis(double available_power, const DeviceOnOff *device) const
{
	double consumption = device->GetExpectedConsumption();

	if(device->GetState())
		return (available_power - consumption > -hysteresis_import); // We are already on, so stay on as long as we have power to offload

	return ((available_power - hysteresis_export) > consumption); // We are off, turn on only if we have enough power to offload
}

bool DevicesManager::force(const map<device::DeviceOnOff *, bool> &devices)
{
	bool state_changed = false;

	// Change all forced devices (no cool down between forced actions)
	for(auto device : devices)
	{
		if(device.first->GetState()!=device.second)
		{
			device.first->SetState(device.second);
			state_changed = true;
		}
	}

	return state_changed;
}

bool DevicesManager::offload(const vector<device::DeviceOnOff *> &devices)
{
	bool state_changed = false;

	double available_power = available_power_avg.Get();

	// Compute theoretical available power with all devices off
	for(auto device : devices)
		available_power += device->GetState()?device->GetExpectedConsumption():0;

	// Offload based of priorities
	for(auto device : devices)
	{
		bool new_state = hysteresis(available_power, device);
		if(new_state)
			available_power -= device->GetExpectedConsumption();

		if(new_state!=device->GetState())
		{
			device->SetState(new_state);
			state_changed = true;
		}
	}

	return state_changed;
}

void DevicesManager::main()
{
	Timestamp last_change_ts;
	Timestamp last_state_update;
	Timestamp last_power_update;

	while(true)
	{
		bool state_changed = false;

		{
			DevicesOnOff devices;

			try
			{
				Timestamp now(TS_MONOTONIC);

				if(now-last_state_update>state_update_interval)
				{
					// Update all devices' state
					for(auto it = devices.begin(); it!=devices.end(); ++it)
						(*it)->UpdateState();

					last_state_update = now;
					state_changed = true;
				}

				// Update devices that have been reloaded or have had a manuel change
				for(auto it = devices.begin(); it!=devices.end(); ++it)
				{
					if((*it)->NeedStateUpdate())
					{
						(*it)->UpdateState();
						state_changed = true;
					}

					if((*it)->WasChanged())
					{
						(*it)->AckChanged();
						state_changed = true;
					}
				}

				// Fetch all devices wanted state
				map<DeviceOnOff *, bool> forced_devices;
				vector<DeviceOnOff *> offload_devices;

				for(auto it = devices.begin(); it!=devices.end(); ++it)
				{
					DeviceOnOff *device = *it;

					en_wanted_state new_state = device->GetWantedState();
					if(new_state==ON || new_state==OFF)
						forced_devices.insert({device, new_state==ON?true:false});
					else if(new_state==OFFLOAD)
						offload_devices.push_back(device);
				}

				// Change all forced devices (no cool down between forced actions)
				state_changed |= force(forced_devices);

				// Apply cooldown time for offload devices
				if(now-last_change_ts>=cooldown && !state_changed)
				{
					// Compute moving average of available power (we dont't want to count during cooldown to let power be accurate)
					available_power_avg.Add(global_meter->GetNetAvailablePower(true), now - last_state_update);
					last_state_update = now;

					state_changed |= offload(offload_devices);
				}

				if(state_changed)
				{
					last_change_ts = now;
					available_power_avg.Reset(); // We have made changes, state is no longer stable
				}
			}
			catch(exception &e)
			{
				logs::Logger::Log(LOG_ERR, e.what());
			}
		}

		if(websocket::SolarControl::GetInstance() && state_changed)
			websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);

		if(!wait(1))
			return;
	}
}

}

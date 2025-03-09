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
#include <device/electrical/DeviceOnOff.hpp>
#include <device/weather/DeviceWeather.hpp>
#include <energy/GlobalMeter.hpp>
#include <websocket/SolarControl.hpp>
#include <control/ConfigurationControl.hpp>
#include <energy/MovingAverage.hpp>
#include <logs/Logger.hpp>

#include <stdexcept>

using namespace std;
using namespace device;
using datetime::Timestamp;

namespace thread {

struct DevicesPtrComparator {
	bool operator()(DeviceOnOff *a, DeviceOnOff *b) const { return a->GetPrio() < b->GetPrio(); }
};

DevicesManager *DevicesManager::instance = 0;

DevicesManager::DevicesManager()
{
	Reload();

	global_meter = energy::GlobalMeter::GetInstance();

	start();

	instance = this;
}

DevicesManager::~DevicesManager()
{
	free();
}

void DevicesManager::Reload()
{
	unique_lock<mutex> llock(lock);

	free();

	auto config = configuration::ConfigurationControl::GetInstance();
	hysteresis_export = config->GetPower("control.hysteresis.export");
	hysteresis_import = config->GetPower("control.hysteresis.import");
	state_update_interval = config->GetTime("control.state.update_interval");
	cooldown = config->GetTime("control.cooldown");

	available_power_avg = new energy::MovingAverage(config->GetTime("control.hysteresis.smoothing"));
}

void DevicesManager::free()
{
	if(available_power_avg)
	{
		delete available_power_avg;
		available_power_avg = 0;
	}
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

	double available_power = available_power_avg->Get();

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
	Timestamp last_change_ts(TS_MONOTONIC);
	Timestamp last_state_update(TS_MONOTONIC);
	Timestamp last_power_update(TS_MONOTONIC);

	while(true)
	{
		Timestamp now(TS_MONOTONIC);
		bool state_changed = false;
		bool state_updated = false;

		// Compute moving average of available power (we don't want to count during cooldown to let power be accurate)
		// global_meter is locked before locking devices (and never locked after)
		if(now-last_change_ts>=cooldown)
			available_power_avg->Add(global_meter->GetNetAvailablePower(true), now - last_power_update);
		last_power_update = now;

		{
			Devices devices;
			auto weather = devices.GetWeather();
			for(auto it = weather.begin(); it!=weather.end(); ++it)
				(*it)->Log();
		}

		{
			Devices devices;
			auto electrical = devices.GetElectrical();
			for(auto it = electrical.begin(); it!=electrical.end(); ++it)
				(*it)->LogEnergy();
		}

		{
			// Lock devices during computations
			Devices devices;

			// Sort OnOff devices by priority
			std::multiset<DeviceOnOff *, DevicesPtrComparator> onoff;
			for(auto device : devices.GetElectrical())
			{
				if(device->GetCategory()==ONOFF)
					onoff.insert((DeviceOnOff *)device);
			}

			// Lock our config
			unique_lock<mutex> llock(lock);

			try
			{

				if(now-last_state_update>state_update_interval)
				{
					// Update all devices' state
					for(auto it = onoff.begin(); it!=onoff.end(); ++it)
					{
						(*it)->UpdateState();
						state_updated = true;
					}

					last_state_update = now;
				}

				// Update devices that have been reloaded or have had a manuel change
				for(auto it = onoff.begin(); it!=onoff.end(); ++it)
				{
					// Reloaded device
					if((*it)->NeedStateUpdate())
					{
						(*it)->UpdateState();
						state_updated = true;
					}

					// Manual change
					if((*it)->WasChanged())
					{
						(*it)->AckChanged();
						state_changed = true;
					}
				}

				// Fetch all devices wanted state
				map<DeviceOnOff *, bool> forced_devices;
				vector<DeviceOnOff *> offload_devices;

				for(auto it = onoff.begin(); it!=onoff.end(); ++it)
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
					state_changed |= offload(offload_devices);

				if(state_changed)
				{
					last_change_ts = now;
					available_power_avg->Reset(); // We have made changes, state is no longer stable
				}
			}
			catch(exception &e)
			{
				logs::Logger::Log(LOG_ERR, e.what());
			}
		}

		if(websocket::SolarControl::GetInstance() && (state_changed || state_updated))
			websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);

		if(!wait(1))
			return;
	}
}

}

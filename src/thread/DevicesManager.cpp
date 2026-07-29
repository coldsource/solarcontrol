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
#include <device/electrical/OnOff.hpp>
#include <energy/GlobalMeter.hpp>
#include <energy/AlgoForced.hpp>
#include <energy/AlgoOffload.hpp>
#include <energy/AlgoOffloadBattery.hpp>
#include <websocket/SolarControl.hpp>
#include <logs/Logger.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <excpt/Shelly.hpp>


using namespace std;
using namespace device;
using datetime::Timestamp;

namespace thread {

DevicesManager *DevicesManager::instance = 0;

DevicesManager::DevicesManager()
{
	// Register as configuration observer and trigger ConfigurationChanged() for initial config loading
	ObserveConfiguration("control");

	global_meter = energy::GlobalMeter::GetInstance();

	instance = this;
}

DevicesManager::~DevicesManager()
{
}

void DevicesManager::ConfigurationChanged(const configuration::ConfigurationPart *config)
{
	unique_lock<mutex> llock(lock);

	cooldown = config->GetTime("control.cooldown");
}

void DevicesManager::main()
{
	Timestamp last_change_ts(TS_MONOTONIC);
	Timestamp last_state_update(TS_MONOTONIC);
	Timestamp last_power_update(TS_MONOTONIC);

	while(true)
	{
		try
		{
			// Lock our config
			unique_lock<mutex> llock(lock);

			Timestamp now(TS_MONOTONIC);
			bool state_changed = false;

			last_power_update = now;

			// We only work on controllable (OnOff) devices
			auto onoff = Devices::Get<OnOff>();

			map<shared_ptr<OnOff>, bool> forced_devices;
			vector<shared_ptr<OnOff>> offload_devices;
			vector<shared_ptr<OnOff>> offload_devices_battery;

			for(auto it = onoff.begin(); it!=onoff.end(); ++it)
			{
				auto device = *it;

				if(device->IsOffline())
					continue; // Ignore offline devices (may be offline or misconfigured)

				if(!device->IsEnabled())
					continue; // Ignore disabled devices

				en_wanted_state new_state = device->GetWantedState();
				if(new_state==ON || new_state==OFF)
					forced_devices.insert({device, new_state==ON?true:false});
				else if(new_state==UNCHANGED)
					forced_devices.insert({device, device->GetState()});
				else if(new_state==OFFLOAD)
				{
					if(device->IsOnBattery())
						offload_devices_battery.push_back(device);
					else
						offload_devices.push_back(device);
				}
			}

			// Change all forced devices (no cool down between forced actions)
			energy::AlgoForced algo_forced(forced_devices);
			algo_forced.Run();
			forced_power = algo_forced.GetEnabledPower();
			state_changed |= algo_forced.HasStateChanged();

			// Apply cooldown time for offload devices
			if((unsigned int)(now-last_change_ts)>=cooldown && !state_changed)
			{
				energy::AlgoOffload algo_offload(offload_devices, forced_power);
				algo_offload.Run();
				state_changed |= algo_offload.HasStateChanged();

				energy::AlgoOffloadBattery algo_offload_battery(offload_devices_battery);
				algo_offload_battery.Run();
				state_changed |= algo_offload_battery.HasStateChanged();
			}

			if(state_changed)
				last_change_ts = now; // Apply new cooldown

			// Handle specific action
			for(auto device : Devices::Get<Device>())
			{
				if(device->IsEnabled()) // Ignore disabled devices
					device->SpecificActions();
			}

		}
		catch(exception &e)
		{
			logs::Logger::Log(LOG_ERR, e.what());
		}

		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::METER);

		if(!wait(1))
			return;
	}
}

}

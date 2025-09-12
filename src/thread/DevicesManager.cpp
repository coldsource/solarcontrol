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
#include <thread/Stats.hpp>
#include <datetime/Timestamp.hpp>
#include <device/Devices.hpp>
#include <device/electrical/DeviceOnOff.hpp>
#include <energy/GlobalMeter.hpp>
#include <websocket/SolarControl.hpp>
#include <logs/Logger.hpp>
#include <configuration/ConfigurationPart.hpp>

#include <stdexcept>

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
	stats = Stats::GetInstance();

	instance = this;
}

DevicesManager::~DevicesManager()
{
}

void DevicesManager::ConfigurationChanged(const configuration::ConfigurationPart *config)
{
	unique_lock<mutex> llock(lock);

	hysteresis_export = config->GetPower("control.hysteresis.export");
	hysteresis_import = config->GetPower("control.hysteresis.import");
	hysteresis_precision = config->GetPercent("control.hysteresis.precision");
	cooldown = config->GetTime("control.cooldown");
}

bool DevicesManager::hysteresis(const shared_ptr<DeviceOnOff> device) const
{
	double consumption = device->GetExpectedConsumption();
	double active_power = forced_power + offloaded_power;

	if(device->GetState())
	{
		double available_power = stats->GetControllablePowerAvg() - active_power;
		return consumption - available_power < hysteresis_import; // We are already on, so stay on as long as we have power to offload
	}

	// Get device minimum on time
	// We need more precision for a device that we can't switch off quickly
	unsigned long long min_on = device->GetMinOn();
	if(min_on==0)
		min_on = 10; // Device has no minimum on time configuration, set a default of 10s

	// We are off, turn on only if we have enough power to offload
	// Look into the past to see the percentage of the time the device could have been on without importing
	// We check for a longer period if the device can't be quickly switched off
	// Compare this to the required precision
	return stats->GetDevicePrediction(device, active_power) >= hysteresis_precision;
}

bool DevicesManager::offload(const vector<shared_ptr<device::DeviceOnOff>> &devices)
{
	bool state_changed = false;
	offloaded_power = 0;

	// Offload based of priorities
	for(auto device : devices)
	{
		bool new_state = hysteresis(device);
		if(new_state)
			offloaded_power += device->GetExpectedConsumption();

		try
		{
			if(new_state!=device->GetState())
			{
				device->SetState(new_state);
				state_changed = true;
			}
		}
		catch(exception &e)
		{
			// Continue even if some deveices have errors, they may simply be offline
			logs::Logger::Log(LOG_ERR, e.what());
		}
	}

	return state_changed;
}

bool DevicesManager::force(const map<shared_ptr<device::DeviceOnOff>, bool> &devices)
{
	bool state_changed = false;
	forced_power = 0;

	// Change all forced devices (no cool down between forced actions)
	for(auto [device, new_state] : devices)
	{
		if(device->GetState()!=new_state)
		{
			device->SetState(new_state);
			state_changed = true;
		}

		if(device->GetState())
			forced_power += device->GetExpectedConsumption();
	}

	return state_changed;
}

double DevicesManager::get_controllable_power() const
{
	auto onoff = Devices::Get<DeviceOnOff>();

	double controlled_active_power = 0;

	for(auto device : onoff)
	{
		if(device->GetState())
			controlled_active_power += device->GetExpectedConsumption();
	}

	return controlled_active_power + global_meter->GetNetAvailablePower(true);
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

			controllable_power = get_controllable_power();

			last_power_update = now;

			// We only work on controllable (OnOff) devices
			auto onoff = Devices::Get<DeviceOnOff>();

			map<shared_ptr<DeviceOnOff>, bool> forced_devices;
			vector<shared_ptr<DeviceOnOff>> offload_devices;

			for(auto it = onoff.begin(); it!=onoff.end(); ++it)
			{
				auto device = *it;

				en_wanted_state new_state = device->GetWantedState();
				if(new_state==ON || new_state==OFF)
					forced_devices.insert({device, new_state==ON?true:false});
				else if(new_state==UNCHANGED)
					forced_devices.insert({device, device->GetState()});
				else if(new_state==OFFLOAD)
					offload_devices.push_back(device);
			}

			// Change all forced devices (no cool down between forced actions)
			state_changed |= force(forced_devices);

			// Apply cooldown time for offload devices
			if((unsigned int)(now-last_change_ts)>=cooldown && !state_changed)
				state_changed |= offload(offload_devices);

			if(state_changed)
				last_change_ts = now; // Apply new cooldown
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

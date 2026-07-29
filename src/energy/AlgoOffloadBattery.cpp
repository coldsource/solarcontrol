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

#include <energy/AlgoOffloadBattery.hpp>
#include <device/electrical/OnOff.hpp>
#include <device/electrical/Battery.hpp>
#include <device/Devices.hpp>
#include <logs/Logger.hpp>
#include <excpt/Shelly.hpp>

using namespace std;
using device::OnOff;
using device::Battery;

namespace energy {

void AlgoOffloadBattery::Run()
{
	state_changed = false;
	offloaded_power = 0;

	auto battery= device::Devices::GetByID<Battery>(DEVICE_ID_BATTERY);

	// Computed forced power on the battery (power that can't be controlled)
	double forced_power = battery->GetPower();
	for(auto device : devices)
	{
		if(device->GetState())
			forced_power -= device->GetExpectedConsumption();
	}

	bool offload_allowed = battery->IsOffloadAllowed();
	double offload_max = (double)battery->GetOffloadMax();

	// Offload based of priorities
	for(auto device : devices)
	{
		bool new_state;

		if(!offload_allowed)
			new_state = false; // Battery offload is disabled, all devices will be set to OFF
		else
		{
			double active_power = forced_power + offloaded_power;

			// Offload until max offload is reached
			new_state = active_power + device->GetExpectedConsumption() < offload_max;
			if(new_state)
				offloaded_power += device->GetExpectedConsumption();
		}

		try
		{
			if(new_state!=device->GetState())
			{
				device->SetState(new_state);
				state_changed = true;
			}
		}
		catch(excpt::Shelly &e)
		{
			// Continue even if some devices have errors, they may simply be offline
			e.Log(LOG_WARNING);
		}
	}
}

}



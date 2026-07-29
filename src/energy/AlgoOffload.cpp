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

#include <energy/AlgoOffload.hpp>
#include <device/electrical/OnOff.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <thread/Stats.hpp>
#include <logs/Logger.hpp>
#include <excpt/Shelly.hpp>

using namespace std;
using device::OnOff;

namespace energy {

AlgoOffload::AlgoOffload(const std::vector<std::shared_ptr<device::OnOff>> &devices, double forced_power):devices(devices), forced_power(forced_power)
{
	ObserveConfiguration("control");

	stats = ::thread::Stats::GetInstance();
}

void AlgoOffload::ConfigurationChanged(const configuration::ConfigurationPart *config)
{
	hysteresis_export = config->GetPower("control.hysteresis.export");
	hysteresis_import = config->GetPower("control.hysteresis.import");
	hysteresis_precision = config->GetPercent("control.hysteresis.precision");
}

bool AlgoOffload::hysteresis(const shared_ptr<OnOff> device) const
{
	double consumption = device->GetExpectedConsumption();
	double active_power = forced_power + offloaded_power;

	if(device->GetState())
	{
		double available_power = stats->GetControllablePowerAvg() - active_power;
		return consumption - available_power < hysteresis_import; // We are already on, so stay on as long as we have power to offload
	}

	// We are off, turn on only if we have enough power to offload
	// Look into the past to see the percentage of the time the device could have been on without importing
	// We check for a longer period if the device can't be quickly switched off
	// Compare this to the required precision
	return stats->GetDevicePrediction(device, active_power) >= hysteresis_precision;
}

void AlgoOffload::Run()
{
	state_changed = false;
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
		catch(excpt::Shelly &e)
		{
			// Continue even if some devices have errors, they may simply be offline
			e.Log(LOG_WARNING);
		}
	}
}

}


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

#include <energy/AlgoForced.hpp>
#include <device/electrical/OnOff.hpp>
#include <logs/Logger.hpp>
#include <excpt/Shelly.hpp>

using namespace std;

namespace energy {

void AlgoForced::Run()
{
	state_changed = false;
	forced_power = 0;

	// Change all forced devices (no cool down between forced actions)
	for(auto [device, new_state] : devices)
	{
		try
		{
			if(device->GetState()!=new_state)
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

		if(device->GetState())
			forced_power += device->GetExpectedConsumption();
	}
}

}

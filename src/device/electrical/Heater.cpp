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

#include <device/electrical/Heater.hpp>

namespace device {

bool Heater::temp_check_force(double current_temp, double timerange_temp) const
{
	if(absence)
		timerange_temp = absence_temperature; // Use special absence temperature if absent mode is on

	return current_temp<timerange_temp;
}

bool Heater::temp_check_offload(double current_temp, double timerange_temp) const
{
	return current_temp<timerange_temp; // In offload mode, we ignore absent mode
}

}

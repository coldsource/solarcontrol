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

#include <energy/Counter.hpp>
#include <energy/Amount.hpp>
#include <datetime/Timestamp.hpp>
#include <datetime/DateTime.hpp>

using namespace std;

namespace energy {

Counter::Counter(int device_id, const string &consumption_history_type, const string &excess_history_type):
consumption_history(device_id, consumption_history_type),
excess_history(device_id, excess_history_type),
consumption_history_detail(device_id, consumption_history_type),
excess_history_detail(device_id, excess_history_type)
{
	last_yday = datetime::DateTime().GetYearDay();

	energy_consumption = consumption_history.GetTotalForToday();
	energy_excess = excess_history.GetTotalForToday();
}

void Counter::AddEnergy(double consumption, double excess)
{
	int yday = datetime::DateTime().GetYearDay();
	if(last_yday!=yday)
	{
		last_yday = yday;

		energy_consumption = 0;
		energy_excess = 0;
	}

	if(consumption>0)
	{
		energy_consumption += consumption;
		consumption_history.Add(consumption);
		consumption_history_detail.Add(consumption);
	}

	if(excess>0)
	{
		energy_excess += excess;
		excess_history.Add(excess);
		excess_history_detail.Add(excess);
	}
}

Amount Counter::GetEnergyConsumption() const
{
	if(last_yday!=datetime::DateTime().GetYearDay())
		return 0;

	return energy_consumption;
}

Amount Counter::GetEnergyExcess() const
{
	if(last_yday!=datetime::DateTime().GetYearDay())
		return 0;

	return energy_excess;
}

}

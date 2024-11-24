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
#include <datetime/Timestamp.hpp>
#include <datetime/DateTime.hpp>

using namespace std;

namespace energy {

Counter::Counter(unsigned int device_id, const string &consumption_history_type, const string &excess_history_type):
consumption_history(consumption_history_type),
excess_history(excess_history_type),
consumption_history_detail(device_id, consumption_history_type),
excess_history_detail(device_id, excess_history_type)
{
	last_ts = 0;
	last_yday = datetime::DateTime().GetYearDay();

	power = 0;
	energy_consumption = consumption_history.GetTotalForToday();
	energy_excess = excess_history.GetTotalForToday();
}

void Counter::SetPower(double v)
{
	power = v;

	int yday = datetime::DateTime().GetYearDay();
	if(last_yday!=yday)
	{
		last_yday = yday;
		last_ts = 0;

		energy_consumption = 0;
		energy_excess = 0;
	}

	double ts = datetime::Timestamp(TS_MONOTONIC);

	if(last_ts!=0 && ts-last_ts<=60)
	{
		double energy_delta = v * (ts-last_ts) / 3600;
		if(power>=0)
		{
			energy_consumption += energy_delta;
			consumption_history.Add(energy_delta);
			consumption_history_detail.Add(energy_delta);
		}
		else
		{
			energy_excess += -energy_delta;
			excess_history.Add(-energy_delta);
			excess_history_detail.Add(-energy_delta);
		}
	}

	last_ts = ts;
}

double Counter::GetPower() const
{
	return power;
}

double Counter::GetEnergyConsumption() const
{
	return energy_consumption;
}

double Counter::GetEnergyExcess() const
{
	return energy_excess;
}

}

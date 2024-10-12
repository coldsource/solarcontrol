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
#include <datetime/Time.hpp>

using namespace std;

namespace energy {

Counter::Counter()
{
	last_ts = 0;
	last_yday = datetime::Time().GetYearDay();

	power = 0;
	energy_consumption = 0;
	energy_excess = 0;
}

void Counter::SetPower(double v)
{
	unique_lock<mutex> llock(lock);

	power = v;

	int yday = datetime::Time().GetYearDay();
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
			energy_consumption += energy_delta;
		else
			energy_excess += -energy_delta;
	}

	last_ts = ts;
}

double Counter::GetPower()
{
	unique_lock<mutex> llock(lock);

	return power;
}

double Counter::GetEnergyConsumption()
{
	unique_lock<mutex> llock(lock);

	return energy_consumption;
}

double Counter::GetEnergyExcess()
{
	unique_lock<mutex> llock(lock);

	return energy_excess;
}

}

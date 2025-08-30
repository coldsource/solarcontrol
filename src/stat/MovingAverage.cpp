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

#include <stat/MovingAverage.hpp>
#include <stdio.h>

using namespace std;

namespace stat {

void MovingAverage::Add(double value, double duration)
{
	// We use integer precision for value and ms for duration
	int64_t ivalue = (int64_t)value;
	int64_t iduration = (int64_t)(duration * 1000);

	if(iduration<=0)
		return;

	data.push_back({ivalue, iduration});
	value_sum += ivalue * iduration;
	duration_sum += iduration;

	while(duration_sum>period && data.size()>1)
	{
		st_point point = data.front();
		value_sum -= point.value * point.duration;
		duration_sum -= point.duration;
		data.pop_front();
	}

	avg = (double)value_sum / (double)duration_sum;
}

double MovingAverage::Get(void) const
{
	return avg;
}

int MovingAverage::GetHigherValuesPercentile(double value, unsigned long max_period) const
{
	int64_t ivalue = (int64_t)value;
	int percentile = 0;

	size_t size = 0;
	int64_t total_period = 0;
	int64_t lmax_period = (int64_t)max_period * 1000;

	for(auto point = data.rbegin(); point!=data.rend(); ++point)
	{
		total_period +=point->duration;

		if(point->value>ivalue)
			percentile++;

		size++;

		if(lmax_period!=0 && total_period>=lmax_period)
			break;
	}

	return percentile * 100 / (int)size;
}

void MovingAverage::Reset()
{
	data.clear();
	value_sum = 0;
	duration_sum = 0;
}

}

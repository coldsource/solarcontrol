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

#ifndef __STAT_MOVINGAVERAGE_HPP__
#define __STAT_MOVINGAVERAGE_HPP__

#include <list>
#include <stdexcept>

namespace stat {

template<typename T>
class MovingAverage
{
	protected:
		struct st_value
		{
			T value;
			double weighting;
		};

		std::list<st_value> values;
		double window_size;

		T last_window_sum = 0;
		double last_windows_size = 0;
		T current_window_sum = 0;
		double current_window_size = 0;

	public:
		MovingAverage(double window_size):window_size(window_size) {}

		void Add(T value, double weighting = 1)
		{
			if(weighting==0)
				weighting = 1; // Can't zero weight one point

			if(current_window_size>=window_size)
			{
				last_window_sum = current_window_sum;
				last_windows_size = current_window_size;
				current_window_sum = 0;
				current_window_size = 0;
			}

			current_window_size += weighting;
			current_window_sum += weighting==1?value:(value * weighting);
			values.push_back({value, weighting});

			while(last_windows_size + current_window_size > window_size && values.size()>1)
			{
				auto last = values.front();
				last_windows_size -= last.weighting;
				last_window_sum -= last.weighting==1?last.value:(last.value * last.weighting);
				values.pop_front();
			}
		}

		T Get() const
		{
			if(last_windows_size + current_window_size==0)
				throw std::runtime_error("Moving Average is empty");

			return (last_window_sum + current_window_sum) / (double)(last_windows_size + current_window_size);
		}

		size_t Size() const { return values.size(); }
};

}

#endif

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

#ifndef __STAT_CUMULATIVEHIGHERFRENQUENCY_HPP__
#define __STAT_CUMULATIVEHIGHERFRENQUENCY_HPP__

#include <map>
#include <list>
#include <vector>
#include <stdexcept>

namespace stat {

template<typename T>
class CumulativeHigherFrequency
{
	protected:
		std::map<T, int> range_size;
		std::list<T> values;
		size_t window_size;

	public:
		CumulativeHigherFrequency(size_t window_size, T min, T max, T step):window_size(window_size)
		{
			for(T i = min; i<=max; i+=step)
				range_size[i] = 0;
		}

		void Add(T value)
		{
			auto it_max = range_size.upper_bound(value);

			for(auto it = range_size.begin(); it!=it_max; ++it)
				it->second++;

			values.push_back(value);

			while(values.size()>1 && values.size()>window_size)
			{
				auto it_max = range_size.upper_bound(values.front());
				for(auto it = range_size.begin(); it!=it_max; ++it)
					it->second--;
				values.pop_front();
			}
		}

		double GetFrequency(T value)
		{
			auto it = range_size.lower_bound(value);
			if(it==range_size.end())
				return (double)range_size.rbegin()->second / (double)values.size();

			return (double)it->second / (double)values.size();
		}

		std::map<T, double> GetFrequencies()
		{
			std::map<T, double> frequencies;
			for(auto [range, size] : range_size)
				frequencies[range] = (double)size / (double)values.size();
			return frequencies;
		}
};

}

#endif

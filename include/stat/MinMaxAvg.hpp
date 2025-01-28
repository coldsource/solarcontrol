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

#ifndef __STAT_MINMAXAVG_HPP__
#define __STAT_MINMAXAVG_HPP__

#include <stat/MinMax.hpp>

namespace stat {

template<typename DataType>
class MinMaxAvg
{
	protected:
		DataType sum;
		unsigned int n;
		MinMax<DataType> minmax;

	public:
		MinMaxAvg(DataType val): sum(val), n(1), minmax(val) {}
		MinMaxAvg(MinMax<DataType> minmax, DataType sum, unsigned int n): sum(sum), n(n), minmax(minmax) {}
		MinMaxAvg(DataType min, DataType max, DataType avg): sum(avg), n(1), minmax(min, max) {}

		MinMaxAvg operator+(const MinMaxAvg &r) const
		{
			return MinMaxAvg(minmax + r.minmax, sum + r.sum, n + r.n);
		}

		MinMaxAvg &operator+=(const MinMaxAvg &r)
		{
			sum += r.sum;
			n += r.n;
			minmax += r.minmax;
			return *this;
		}

		DataType GetMin() const { return minmax.GetMin(); }
		DataType GetMax() const { return minmax.GetMax(); }
		DataType GetAvg() const { return sum / n; }
};

}

#endif


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

#ifndef __STAT_MINMAX_HPP__
#define __STAT_MINMAX_HPP__

namespace stat {

template <typename T>
concept LessGreaterThan = requires (T val1, T val2) {
  val1 < val2;
  val1 > val2;
};

template<LessGreaterThan DataType>
class MinMax
{
	protected:
		DataType min;
		DataType max;

	public:
		MinMax(DataType val): min(val), max(val) {}
		MinMax(DataType min, DataType max): min(min), max(max) {}

		MinMax operator+(const MinMax &r) const
		{
			DataType new_min = min<r.min?min:r.min;
			DataType new_max = max>r.max?max:r.max;
			return MinMax(new_min, new_max);
		}

		MinMax &operator+=(const MinMax &r)
		{
			min = min<r.min?min:r.min;
			max = max>r.max?max:r.max;
			return *this;
		}

		DataType GetMin() const { return min; }
		DataType GetMax() const { return max; }
};

}

#endif

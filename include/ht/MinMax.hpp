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

#ifndef __HT_MINMAX_HPP__
#define __HT_MINMAX_HPP__

#include <stat/MinMax.hpp>

#include <limits>

namespace ht {

class MinMax
{
	stat::MinMax<double> h;
	stat::MinMax<double> t;
	stat::MinMax<double> w;

	public:
		MinMax(double h, double t): h(h), t(t), w(std::numeric_limits<double>::quiet_NaN()) {}
		MinMax(double w): h(std::numeric_limits<double>::quiet_NaN()), t(std::numeric_limits<double>::quiet_NaN()), w(w) {}
		MinMax(stat::MinMax<double> h, stat::MinMax<double> t, stat::MinMax<double> w): h(h), t(t), w(w) {}

		stat::MinMax<double> GetHumidity() const { return h; }
		stat::MinMax<double> GetTemperature() const { return t; }
		stat::MinMax<double> GetWind() const { return w; }

		MinMax operator+(const MinMax &r) const;
		MinMax &operator+=(const MinMax &r);
};

}

#endif

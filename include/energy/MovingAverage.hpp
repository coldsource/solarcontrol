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

#ifndef __ENERGY_MOVINGAVERAGE_HPP__
#define __ENERGY_MOVINGAVERAGE_HPP__

#include <list>

namespace energy {

class MovingAverage
{
	protected:
		// Store values in integer to avoid rounding errors
		struct st_point
		{
			int value;
			int duration;
		};

		int value_sum = 0;
		int duration_sum = 0;
		double avg = 0;
		int period;

		std::list<st_point> data;

	public:
		MovingAverage(int period): period(period * 1000) {}

		void Add(double value, double duration);
		double Get() const;

		void Reset();
};

}

#endif



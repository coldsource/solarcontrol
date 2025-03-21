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

#ifndef __METER_METER_HPP__
#define __METER_METER_HPP__

#include <mutex>
#include <atomic>

namespace configuration {
	class Json;
}

namespace meter {

class Meter
{
	protected:
		std::atomic<double> power = -1;
		double energy_consumption = 0;
		double energy_excess = 0;

		std::mutex lock;

	public:
		virtual ~Meter() {}

		static void CheckConfig(const configuration::Json &conf);

		double GetPower() const;
		double GetConsumption();
		double GetExcess();
};

}

#endif

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

#ifndef __SENSOR_WEATHER_HT_HPP__
#define __SENSOR_WEATHER_HT_HPP__

#include <sensor/weather/Weather.hpp>

#include <atomic>
#include <limits>

namespace configuration {
	class Json;
}

namespace sensor::weather {

class HT: public Weather
{
	protected:
		std::atomic<double> temperature = std::numeric_limits<double>::quiet_NaN();
		std::atomic<double> humidity = std::numeric_limits<double>::quiet_NaN();

	public:
		virtual ~HT() {}

		static void CheckConfig(const configuration::Json & /* conf */) {}

		double GetTemperature() const { return temperature; }
		double GetHumidity() const { return humidity; }
};

}

#endif



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

#ifndef __SENSOR_WEATHER_WIND_HPP__
#define __SENSOR_WEATHER_WIND_HPP__

#include <sensor/Sensor.hpp>
#include <mqtt/Subscriber.hpp>

#include <atomic>
#include <limits>

namespace configuration {
	class Json;
}

namespace sensor::weather {

class Wind: public Sensor, public mqtt::Subscriber
{
	const std::string topic;

	std::atomic<double> wind = std::numeric_limits<double>::quiet_NaN();

	public:
		Wind(const std::string &mqtt_id);
		virtual ~Wind();

		static void CheckConfig(const configuration::Json &conf);

		double GetWind() const;
		void SetWind(double w);

		void HandleMessage(const std::string &message, const std::string & /*topic*/) override;
};

}

#endif




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

#ifndef __SENSOR_WEATHER_HTBLUETOOTH_HPP__
#define __SENSOR_WEATHER_HTBLUETOOTH_HPP__

#include <sensor/weather/HT.hpp>
#include <mqtt/Subscriber.hpp>

#include <string>

namespace sensor::weather {

class HTBluetooth: public HT, public mqtt::Subscriber
{
	std::string topic;

	public:
		HTBluetooth(const std::string &ble_addr);
		virtual ~HTBluetooth();

		static void CheckConfig(const configuration::Json &conf);

		void HandleMessage(const std::string &message, const std::string & /*topic*/) override;
};

}

#endif




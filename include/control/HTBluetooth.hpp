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

#ifndef __CONTROL_HTBLUETOOTH_HPP__
#define __CONTROL_HTBLUETOOTH_HPP__

#include <mqtt/Subscriber.hpp>

#include <string>
#include <mutex>

namespace control {

class HTBluetooth: public mqtt::Subscriber
{
	std::string topic;

	double temperature = std::numeric_limits<double>::quiet_NaN();
	double humidity = std::numeric_limits<double>::quiet_NaN();

	mutable std::mutex lock;

	public:
		HTBluetooth(const std::string &ble_addr);
		virtual ~HTBluetooth();

		double GetTemperature() const;
		double GetHumidity() const;

		void HandleMessage(const std::string &message);
};

}

#endif




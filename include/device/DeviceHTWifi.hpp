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

#ifndef __DEVICE_DEVICEHTWIFI_HPP__
#define __DEVICE_DEVICEHTWIFI_HPP__

#include <device/DeviceHT.hpp>
#include <control/HTWifi.hpp>
#include <nlohmann/json.hpp>

#include <string>

namespace configuration {
	class Json;
}

namespace device {

class DeviceHTWifi: public DeviceHT
{
	control::HTWifi *ctrl;

	public:
		DeviceHTWifi(unsigned int id, const std::string &name, const configuration::Json &config);
		virtual ~DeviceHTWifi();

		std::string GetType() const { return "ht"; }

		double GetTemperature() const;
		double GetHumidity() const;
};

}

#endif



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

#ifndef __DEVICE_DEVICEWEATHER_HPP__
#define __DEVICE_DEVICEWEATHER_HPP__

#include <device/Device.hpp>

namespace device {

class DeviceWeather: public Device
{
	public:
		DeviceWeather(unsigned int id, const std::string &name, const configuration::Json &config): Device(id, name, config) {}
		virtual ~DeviceWeather() {}

		en_category GetCategory() const { return WEATHER; }

		virtual double GetTemperature() const = 0;
		virtual double GetHumidity() const = 0;
		virtual double GetWind() const = 0;

		virtual void Log() = 0;
};

}

#endif





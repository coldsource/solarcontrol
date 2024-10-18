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

#ifndef __DEVICE_DEVICEHT_HPP__
#define __DEVICE_DEVICEHT_HPP__

#include <device/Device.hpp>
#include <control/HT.hpp>
#include <nlohmann/json.hpp>

#include <string>

namespace device {

class DeviceHT: public Device
{
	control::HT *ctrl;

	public:
		DeviceHT(unsigned int id, const std::string &name, const nlohmann::json &config);
		virtual ~DeviceHT();

		std::string GetType() const { return "ht"; }

		double GetTemperature() const;
		double GetHumidity() const;
};

}

#endif


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

#ifndef __DEVICE_DEVICEPASSIVE_HPP__
#define __DEVICE_DEVICEPASSIVE_HPP__

#include <device/electrical/DeviceElectrical.hpp>

#include <string>

namespace device {

class DevicePassive: public DeviceElectrical
{
	public:
		DevicePassive(unsigned int id, const std::string &name, const configuration::Json &config): DeviceElectrical(id, name, config) {}

		static void CheckConfig(const configuration::Json &conf);

		en_category GetCategory() const { return PASSIVE; }
		std::string GetType() const { return "passive"; }
};

}

#endif



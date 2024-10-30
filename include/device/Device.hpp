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

#ifndef __DEVICE_DEVICE_HPP__
#define __DEVICE_DEVICE_HPP__

#include <nlohmann/json.hpp>

#include <string>

namespace device {

class Device
{
	unsigned int id;
	std::string name;

	public:
		Device(unsigned int id, const std::string &name);
		Device(const Device&) = delete;
		virtual ~Device() {}

		virtual std::string GetType() const = 0;

		unsigned int GetID() const { return id; }
		std::string GetName() const { return name; }
};

}

#endif

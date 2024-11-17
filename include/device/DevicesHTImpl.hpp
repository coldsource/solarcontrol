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

#ifndef __DEVICE_DEVICESHTIMPL_HPP__
#define __DEVICE_DEVICESHTIMPL_HPP__

#include <device/DeviceHT.hpp>

#include <unordered_set>
#include <mutex>

namespace device {

class DevicesHT;
class Devices;

class DevicesHTImpl: public std::unordered_set<DeviceHT *>
{
	friend class DevicesHT;
	friend class Devices;

	static DevicesHTImpl *instance;

	std::map<unsigned int, DeviceHT *> id_device;

	mutable std::recursive_mutex d_mutex;

	void free();

	public:
		DevicesHTImpl();
		~DevicesHTImpl();

	private:
		DeviceHT *get_by_id(unsigned int id) const;
		void reload(bool notify = true);
		void unload();
};

}

#endif



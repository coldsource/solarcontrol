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

#ifndef __DEVICE_DEVICESPASSIVEIMPL_HPP__
#define __DEVICE_DEVICESPASSIVEIMPL_HPP__

#include <unordered_set>
#include <map>
#include <mutex>

namespace device {

class DevicesPassive;
class Devices;
class DevicePassive;

class DevicesPassiveImpl: public std::unordered_set<DevicePassive *>
{
	friend class DevicesPassive;
	friend class Devices;

	static DevicesPassiveImpl *instance;

	std::map<unsigned int, DevicePassive *> id_device;
	unsigned int hws_id;

	mutable std::recursive_mutex d_mutex;

	void free();

	public:
		DevicesPassiveImpl();
		~DevicesPassiveImpl();

	private:
		DevicePassive *get_by_id(unsigned int id) const;
		void reload(bool notify = true);
		void unload();
};

}

#endif



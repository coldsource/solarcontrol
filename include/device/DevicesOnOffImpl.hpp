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

#ifndef __DEVICE_DEVICESONOFFIMPL_HPP__
#define __DEVICE_DEVICESONOFFIMPL_HPP__

#include <device/DeviceOnOff.hpp>

#include <set>
#include <map>
#include <mutex>

namespace device {

class DevicesOnOff;
class Devices;

struct DevicesPtrComparator {
	bool operator()(DeviceOnOff *a, DeviceOnOff *b) const
	{
		return a->GetPrio() < b->GetPrio();
	}
};

class DevicesOnOffImpl: public std::multiset<DeviceOnOff *, DevicesPtrComparator>
{
	friend class DevicesOnOff;
	friend class Devices;

	static DevicesOnOffImpl *instance;

	std::map<unsigned int, DeviceOnOff *> id_device;

	mutable std::recursive_mutex d_mutex;

	void free();

	public:
		DevicesOnOffImpl();
		~DevicesOnOffImpl();

	private:
		DeviceOnOff *get_by_id(unsigned int id) const;
		void reload(bool notify = true);
		void unload();
};

}

#endif


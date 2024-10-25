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

#ifndef __DEVICE_DEVICESHT_HPP__
#define __DEVICE_DEVICESHT_HPP__

#include <device/DeviceHT.hpp>

#include <unordered_set>
#include <mutex>

namespace device {

class DevicesHT: public std::unordered_set<DeviceHT *>
{
	static DevicesHT *instance;

	std::map<unsigned int, DeviceHT *> id_device;

	mutable std::mutex d_mutex;

	void free();

	public:
		DevicesHT();
		~DevicesHT();

		static DevicesHT *GetInstance() { return instance; }

		DeviceHT *GetByID(unsigned int id) const;

		void Lock() { d_mutex.lock(); }
		void Unlock() { d_mutex.unlock(); }

		void Reload();
};

}

#endif



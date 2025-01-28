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

#ifndef __DEVICE_DEVICESWEATHERIMPL_HPP__
#define __DEVICE_DEVICESWEATHERIMPL_HPP__

#include <unordered_set>
#include <mutex>
#include <map>

namespace device {

class DevicesWeather;
class Devices;
class DeviceWeather;

class DevicesWeatherImpl: public std::unordered_set<DeviceWeather *>
{
	friend class DevicesWeather;
	friend class Devices;

	static DevicesWeatherImpl *instance;

	std::map<unsigned int, DeviceWeather *> id_device;

	mutable std::recursive_mutex d_mutex;

	void free();

	public:
		DevicesWeatherImpl();
		~DevicesWeatherImpl();

	private:
		DeviceWeather *get_by_id(unsigned int id) const;
		void reload(bool notify = true);
		void unload();
};

}

#endif



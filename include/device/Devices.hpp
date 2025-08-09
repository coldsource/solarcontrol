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

#ifndef __DEVICE_DEVICES_HPP__
#define __DEVICE_DEVICES_HPP__

#include <string>
#include <map>
#include <set>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>

#define DEVICE_ID_GRID     -1
#define DEVICE_ID_PV       -2
#define DEVICE_ID_HWS      -3
#define DEVICE_ID_BATTERY  -4

#define DEVICE_NAME_GRID    "grid"
#define DEVICE_NAME_PV      "pv"
#define DEVICE_NAME_HWS     "hws"
#define DEVICE_NAME_BATTERY "battery"

namespace device {

class Device;
class DeviceElectrical;
class DeviceWeather;
class DeviceObserver;

class Devices
{
	static Devices *instance;
	static std::mutex mutex_w;
	static std::shared_mutex mutex_r;

	static std::map<int, Device *> devices;
	static std::unordered_set<DeviceElectrical *> devices_electrical;
	static std::unordered_set<DeviceWeather *> devices_weather;

	static std::map<int, std::set<DeviceObserver *>> observers;

	void lock_write();
	void unlock_write();

	Device *get_by_id(int id) const;
	void reload(int id = 0);

	public:
		Devices();
		~Devices();

		void Reload(int id = 0);
		std::set<Device *> Unload(int id = 0);

		std::string IDToName(int id) const;
		DeviceElectrical *GetElectricalByID(int id) const;
		DeviceWeather *GetWeatherByID(int id) const;

		const std::unordered_set<DeviceElectrical *> &GetElectrical() const { return devices_electrical; }
		const std::unordered_set<DeviceWeather *> &GetWeather() const { return devices_weather; }

		Device *IsInUse(int device_id) const;

		void RegisterObserver(int device_id, DeviceObserver *observer);
		void UnregisterObserver(int device_id, DeviceObserver *observer);
};

}

#endif


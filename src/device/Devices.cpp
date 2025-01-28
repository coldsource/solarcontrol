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

#include <device/Devices.hpp>
#include <device/DevicesOnOffImpl.hpp>
#include <device/DevicesWeatherImpl.hpp>
#include <device/DevicesPassiveImpl.hpp>
#include <device/DeviceOnOff.hpp>
#include <device/DevicePassive.hpp>
#include <device/DeviceWeather.hpp>

#include <mutex>

using namespace std;

namespace device
{

Devices *Devices::instance = 0;

Devices::Devices()
{
	devices_onoff = new DevicesOnOffImpl();
	devices_weather = new DevicesWeatherImpl();
	devices_passive = new DevicesPassiveImpl();

	instance = this;
}

Devices::~Devices()
{
	delete devices_onoff;
	delete devices_weather;
	delete devices_passive;
}

string Devices::IDToName(int id)
{
	// Special devices
	if(id==DEVICE_ID_GRID)
		return DEVICE_NAME_GRID;
	else if(id==DEVICE_ID_PV)
		return DEVICE_NAME_PV;
	else if(id==DEVICE_ID_HWS)
		return DEVICE_NAME_HWS;

	// Lookup standard devices
	try
	{
		unique_lock<recursive_mutex> llock(devices_onoff->d_mutex);
		return devices_onoff->get_by_id(id)->GetName();
	}
	catch(...) {}

	try
	{
		unique_lock<recursive_mutex> llock(devices_weather->d_mutex);
		return devices_weather->get_by_id(id)->GetName();
	}
	catch(...) {}

	try
	{
		unique_lock<recursive_mutex> llock(devices_passive->d_mutex);
		return devices_passive->get_by_id(id)->GetName();
	}
	catch(...) {}

	return ""; // Unknown device
}

void Devices::Reload()
{
	devices_onoff->d_mutex.lock();
	devices_onoff->reload();
	devices_onoff->d_mutex.unlock();

	devices_weather->d_mutex.lock();
	devices_weather->reload();
	devices_weather->d_mutex.unlock();

	devices_passive->d_mutex.lock();
	devices_passive->reload();
	devices_passive->d_mutex.unlock();

}

void Devices::Unload()
{
	devices_onoff->unload();
	devices_weather->unload();
	devices_passive->unload();
}

}

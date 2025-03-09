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
#include <device/DeviceFactory.hpp>
#include <device/Device.hpp>
#include <device/DeviceOnOff.hpp>
#include <device/DeviceWeather.hpp>
#include <device/DeviceObserver.hpp>
#include <configuration/Json.hpp>
#include <database/DB.hpp>
#include <logs/Logger.hpp>
#include <websocket/SolarControl.hpp>

#include <stdexcept>

using namespace std;

namespace device
{

Devices *Devices::instance = 0;
mutex Devices::mutex_w;
shared_mutex Devices::mutex_r;

map<int, Device *> Devices::devices;
unordered_set<DeviceElectrical *> Devices::devices_electrical;
unordered_set<DeviceWeather *> Devices::devices_weather;

map<int, set<DeviceObserver *>> Devices::observers;

Devices::Devices()
{
	if(instance==0)
	{
		reload();
		instance = this;
		return;
	}

	// Read lock mode
	mutex_w.lock();
	mutex_r.lock_shared();
	mutex_w.unlock();
}

Devices::~Devices()
{
	mutex_r.unlock_shared();
}

void Devices::lock_write()
{
	mutex_r.unlock_shared();

	mutex_w.lock();
	mutex_r.lock();
	mutex_w.unlock();
}

void Devices::unlock_write()
{
	mutex_r.unlock();

	mutex_w.lock();
	mutex_r.lock_shared();
	mutex_w.unlock();
}

void Devices::Reload(int id)
{
	// Switch to write locking mode
	lock_write();

	reload(id);

	// Switch back to read locking mode
	unlock_write();
}

void Devices::reload(int id)
{
	set<Device *> old_devices;

	try
	{
		if(id==0)
			logs::Logger::Log(LOG_NOTICE, "Loading devices");
		else
			logs::Logger::Log(LOG_NOTICE, "Reloading device " + to_string(id));

		old_devices = Unload(id);

		database::DB db;

		string WHERE = "";
		if(id!=0)
			WHERE = " WHERE device_id = " + to_string(id);
		database::Query q("SELECT device_id, device_name, device_type, device_config FROM t_device" + WHERE);

		auto res = db.Query(q);
		while(res.FetchRow())
		{
			configuration::Json config((string)res["device_config"]);

			int device_id = res["device_id"];
			Device *device = DeviceFactory::Get(device_id, res["device_name"], res["device_type"], config);

			auto it = observers.find(device_id);
			if(it!=observers.end())
			{
				// Notify observers that device has changed before removing old one
				for(auto observer : it->second)
					observer->DeviceChanged(device);
			}

			devices.insert(pair<int, Device *>(device->GetID(), device));
			if(device->GetCategory()==ONOFF || device->GetCategory()==PASSIVE)
				devices_electrical.insert((DeviceOnOff *)device);
			if(device->GetCategory()==WEATHER)
				devices_weather.insert((DeviceWeather *)device);
		}

		if(websocket::SolarControl::GetInstance())
			websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
	}
	catch(exception &e)
	{
		logs::Logger::Log(LOG_NOTICE, "Error reloading devices : « " + string(e.what()) + " »");
	}

	// Observers have been notified we can now safely remove old devices
	for(auto old_device : old_devices)
		delete old_device;
}

set<Device *> Devices::Unload(int id)
{
	// No locking here as it's only called by Reload or main thread (which is not locked)

	set<Device *> old_devices;

	if(id!=0)
	{
		auto it = devices.find(id);
		if(it==devices.end())
			return old_devices; // No error if device is not found : it's a creation

		Device *device = it->second;
		if(device->GetCategory()==ONOFF || device->GetCategory()==PASSIVE)
			devices_electrical.erase((DeviceElectrical *)device);
		else if(device->GetCategory()==WEATHER)
			devices_weather.erase((DeviceWeather *)device);
		devices.erase(device->GetID());

		old_devices.insert(device);
		return old_devices;
	}

	// Unload and delete all devices objects
	for(auto it = devices.begin(); it!=devices.end(); ++it)
	{
		Device *device = it->second;
		old_devices.insert(device);
	}

	devices.clear();
	devices_electrical.clear();
	devices_weather.clear();
	return old_devices;
}


string Devices::IDToName(int id) const
{
	// Special devices
	if(id==DEVICE_ID_GRID)
		return DEVICE_NAME_GRID;
	else if(id==DEVICE_ID_PV)
		return DEVICE_NAME_PV;
	else if(id==DEVICE_ID_HWS)
		return DEVICE_NAME_HWS;

	auto it = devices.find(id);
	if(it!=devices.end())
		return it->second->GetName();

	return ""; // Unknown device
}

Device *Devices::get_by_id(int id) const
{
	auto it = devices.find(id);
	if(it==devices.end())
		throw out_of_range("Unknown device id : " + to_string(id));

	return it->second;
}

DeviceElectrical *Devices::GetElectricalByID(int id) const
{
	Device *device = get_by_id(id);
	if(device->GetCategory()!=ONOFF && device->GetCategory()!=PASSIVE)
		throw runtime_error("Not an OnOff device : " + to_string(id));
	return (DeviceElectrical *)device;
}

DeviceWeather *Devices::GetWeatherByID(int id) const
{
	Device *device = get_by_id(id);
	if(device->GetCategory()!=WEATHER)
		throw runtime_error("Not a Weather device : " + to_string(id));
	return (DeviceWeather *)device;
}

Device *Devices::IsInUse(int device_id) const
{
	for(auto device : devices)
	{
		if(device.second->Depends(device_id))
			return device.second;
	}

	return 0;
}

void Devices::RegisterObserver(int device_id, DeviceObserver *observer)
{
	lock_write();

	observer->DeviceChanged(get_by_id(device_id));
	observers[device_id].insert(observer);

	unlock_write();
}

void Devices::UnregisterObserver(int device_id, DeviceObserver *observer)
{
	auto it = observers.find(device_id);
	if(it==observers.end())
		return;

	it->second.erase(observer);

	if(it->second.size()==0)
		observers.erase(device_id);
}

}

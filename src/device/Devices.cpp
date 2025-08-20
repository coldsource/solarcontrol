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
#include <device/electrical/DeviceElectrical.hpp>
#include <device/weather/DeviceWeather.hpp>
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

map<int, shared_ptr<Device>> Devices::devices;
unordered_set<shared_ptr<DeviceElectrical>> Devices::devices_electrical;
unordered_set<shared_ptr<DeviceWeather>> Devices::devices_weather;

map<int, set<DeviceObserver *>> Devices::observers;

Devices::Devices()
{
	if(instance==0)
	{
		Reload();
		instance = this;
		return;
	}
}

Devices::~Devices()
{
}

void Devices::Reload(int id)
{
	unique_lock<mutex> llock(mutex_w);

	try
	{
		if(id==0)
			logs::Logger::Log(LOG_NOTICE, "Loading devices");
		else
			logs::Logger::Log(LOG_NOTICE, "Reloading device " + to_string(id));

		Unload(id);

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
			auto device = DeviceFactory::Get(device_id, res["device_name"], res["device_type"], config);

			auto it = observers.find(device_id);
			if(it!=observers.end())
			{
				// Notify observers that device has changed before removing old one
				for(auto observer : it->second)
					observer->DeviceChanged(device);
			}

			devices.insert(pair<int, shared_ptr<Device>>(device->GetID(), device));
			if(device->GetCategory()==ONOFF || device->GetCategory()==PASSIVE)
				devices_electrical.insert(dynamic_pointer_cast<DeviceElectrical>(device));
			if(device->GetCategory()==WEATHER)
				devices_weather.insert(dynamic_pointer_cast<DeviceWeather>(device));
		}

		if(websocket::SolarControl::GetInstance())
			websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
	}
	catch(exception &e)
	{
		logs::Logger::Log(LOG_NOTICE, "Error reloading devices : « " + string(e.what()) + " »");
	}
}

void Devices::Unload(int id)
{
	// No locking here as it's only called by Reload or main thread (which is not locked)

	if(id!=0)
	{
		auto it = devices.find(id);
		if(it==devices.end())
			return; // No error if device is not found : it's a creation

		auto device = it->second;
		if(device->GetCategory()==ONOFF || device->GetCategory()==PASSIVE)
			devices_electrical.erase(dynamic_pointer_cast<DeviceElectrical>(device));
		else if(device->GetCategory()==WEATHER)
			devices_weather.erase(dynamic_pointer_cast<DeviceWeather>(device));
		devices.erase(device->GetID());

		return;
	}

	// Unload and delete all devices objects
	devices.clear();
	devices_electrical.clear();
	devices_weather.clear();
}


string Devices::IDToName(int id) const
{
	unique_lock<mutex> llock(mutex_w);

	// Special devices
	if(id==DEVICE_ID_GRID)
		return DEVICE_NAME_GRID;
	else if(id==DEVICE_ID_PV)
		return DEVICE_NAME_PV;
	else if(id==DEVICE_ID_HWS)
		return DEVICE_NAME_HWS;
	else if(id==DEVICE_ID_BATTERY)
		return DEVICE_NAME_BATTERY;

	auto it = devices.find(id);
	if(it!=devices.end())
		return it->second->GetName();

	return ""; // Unknown device
}

shared_ptr<Device> Devices::get_by_id(int id) const
{
	auto it = devices.find(id);
	if(it==devices.end())
		throw out_of_range("Unknown device id : " + to_string(id));

	return it->second;
}

shared_ptr<DeviceElectrical> Devices::GetElectricalByID(int id) const
{
	unique_lock<mutex> llock(mutex_w);

	auto device = get_by_id(id);
	if(device->GetCategory()!=ONOFF && device->GetCategory()!=PASSIVE)
		throw runtime_error("Not an OnOff device : " + to_string(id));
	return dynamic_pointer_cast<DeviceElectrical>(device);
}

shared_ptr<DeviceWeather> Devices::GetWeatherByID(int id) const
{
	unique_lock<mutex> llock(mutex_w);

	auto device = get_by_id(id);
	if(device->GetCategory()!=WEATHER)
		throw runtime_error("Not a Weather device : " + to_string(id));
	return dynamic_pointer_cast<DeviceWeather>(device);
}

const unordered_set<shared_ptr<DeviceElectrical>> Devices::GetElectrical() const
{
	unique_lock<mutex> llock(mutex_w);

	return devices_electrical;
}

const unordered_set<shared_ptr<DeviceWeather>> Devices::GetWeather() const
{
	unique_lock<mutex> llock(mutex_w);

	return devices_weather;
}

shared_ptr<Device> Devices::IsInUse(int device_id) const
{
	unique_lock<mutex> llock(mutex_w);

	for(auto device : devices)
	{
		if(device.second->Depends(device_id))
			return device.second;
	}

	return nullptr;
}

void Devices::RegisterObserver(int device_id, DeviceObserver *observer)
{
	unique_lock<mutex> llock(mutex_w);

	observer->DeviceChanged(get_by_id(device_id));
	observers[device_id].insert(observer);
}

void Devices::UnregisterObserver(int device_id, DeviceObserver *observer)
{
	unique_lock<mutex> llock(mutex_w);

	auto it = observers.find(device_id);
	if(it==observers.end())
		return;

	it->second.erase(observer);

	if(it->second.size()==0)
		observers.erase(device_id);
}

}

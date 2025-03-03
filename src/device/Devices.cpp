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
#include <device/DeviceTimeRange.hpp>
#include <device/DeviceHeater.hpp>
#include <device/DeviceCMV.hpp>
#include <device/DeviceHWS.hpp>
#include <device/DevicePassive.hpp>
#include <device/DeviceHTWifi.hpp>
#include <device/DeviceHTBluetooth.hpp>
#include <device/DeviceWind.hpp>
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
unordered_set<DeviceOnOff *> Devices::devices_onoff;
unordered_set<DevicePassive *> Devices::devices_passive;
unordered_set<DeviceWeather *> Devices::devices_weather;

Devices::Devices()
{
	if(instance==0)
	{
		reload();
		instance = this;
		return;
	}

	// Read lock mode
	instance->mutex_w.lock();
	instance->mutex_r.lock_shared();
	instance->mutex_w.unlock();
}

Devices::~Devices()
{
	instance->mutex_r.unlock_shared();
}

void Devices::lock_write()
{
	instance->mutex_r.unlock_shared();

	instance->mutex_w.lock();
	instance->mutex_r.lock();
	instance->mutex_w.unlock();
}

void Devices::unlock_write()
{
	instance->mutex_r.unlock();

	instance->mutex_w.lock();
	instance->mutex_r.lock_shared();
	instance->mutex_w.unlock();
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

			Device *device;
			string device_type = res["device_type"];
			if(device_type=="timerange")
				device = new DeviceTimeRange(res["device_id"], res["device_name"], config);
			else if(device_type=="heater")
				device = new DeviceHeater(res["device_id"], res["device_name"], config);
			else if(device_type=="cmv")
				device = new DeviceCMV(res["device_id"], res["device_name"], config);
			else if(device_type=="hws")
				device = new DeviceHWS(res["device_id"], res["device_name"], config);
			else if(device_type=="passive")
				device = new DevicePassive(res["device_id"], res["device_name"], config);
			else if(device_type=="ht")
				device = new DeviceHTWifi(res["device_id"], res["device_name"], config);
			else if(device_type=="htmini")
				device = new DeviceHTBluetooth(res["device_id"], res["device_name"], config);
			else if(device_type=="wind")
				device = new DeviceWind(res["device_id"], res["device_name"], config);
			else
				throw invalid_argument("Invalid device type « " + string(res["device_type"]) + " »");

			devices.insert(pair<int, Device *>(device->GetID(), device));
			if(device->GetCategory()==ONOFF)
				devices_onoff.insert((DeviceOnOff *)device);
			else if(device->GetCategory()==PASSIVE)
				devices_passive.insert((DevicePassive *)device);
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
}

void Devices::Unload(int id)
{
	// No locking here as it's only called by Reload or main thread (which is not locked)

	if(id!=0)
	{
		auto it = devices.find(id);
		if(it==devices.end())
			return; // No error if device is not found : it's a creation

		Device *device = it->second;
		if(device->GetCategory()==ONOFF)
			devices_onoff.erase((DeviceOnOff *)device);
		else if(device->GetCategory()==PASSIVE)
			devices_passive.erase((DevicePassive *)device);
		else if(device->GetCategory()==WEATHER)
			devices_weather.erase((DeviceWeather *)device);
		devices.erase(device->GetID());

		delete device;
		return;
	}

	// Unload and delete all devices objects
	for(auto it = devices.begin(); it!=devices.end(); ++it)
	{
		Device *device = it->second;
		delete device;
	}

	devices.clear();
	devices_onoff.clear();
	devices_passive.clear();
	devices_weather.clear();
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

DeviceOnOff *Devices::GetOnOffByID(int id) const
{
	Device *device = get_by_id(id);
	if(device->GetCategory()!=ONOFF)
		throw runtime_error("Not an OnOff device : " + to_string(id));
	return (DeviceOnOff *)device;
}

DevicePassive *Devices::GetPassiveByID(int id) const
{
	Device *device = get_by_id(id);
	if(device->GetCategory()!=PASSIVE)
		throw runtime_error("Not a Passive device : " + to_string(id));
	return (DevicePassive *)device;
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

}

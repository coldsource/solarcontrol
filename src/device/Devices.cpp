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

Devices::Devices()
{
	if(instance==0)
	{
		try
		{
			database::DB db;

			database::Query q(" \
				SELECT DEV.device_id, DEV.device_name, DEV.device_type, DEV.device_config, STATE.device_state \
				FROM t_device DEV \
				LEFT JOIN t_device_state STATE ON DEV.device_id=STATE.device_id \
				");
			auto res = db.Query(q);
			while(res.FetchRow())
			{
				auto device = Load(res["device_id"], res["device_name"], res["device_type"], configuration::Json((string)res["device_config"]));
				if(!res["device_state"].IsNull())
					device->StateRestore(configuration::Json((string)res["device_state"]));
			}

			instance = this;
			return;
		}
		catch(exception &e)
		{
			logs::Logger::Log(LOG_ERR, "Error loading devices : « " + string(e.what()) + " »");
		}
	}
}

Devices::~Devices()
{
}

std::shared_ptr<Device> Devices::Load(int id, const string &name, const string &type, const configuration::Json &config)
{
	auto device = DeviceFactory::Get(id, name, type, config);

	devices.insert(pair<int, shared_ptr<Device>>(device->GetID(), device));

	return device;
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
			string name = res["device_name"];

			auto device = get_by_id(device_id);
			device->Reload(name, config);
		}

		// On startup, Websocket server is not yet started
		if(websocket::SolarControl::GetInstance())
			websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
	}
	catch(exception &e)
	{
		logs::Logger::Log(LOG_NOTICE, "Error reloading devices : « " + string(e.what()) + " »");
	}
}

void Devices::Unload()
{
	// No locking here as it's only called by Reload or main thread (which is not locked)

	// First backup states
	database::DB db;

	try
	{
		for(auto device : devices)
		{
			auto state = device.second->StateBackup();
			db.Query("REPLACE INTO t_device_state(device_id, device_state) VALUES(%i, %s)"_sql << device.second->GetID() << state.ToString());
		}
	}
	catch(exception &e)
	{
		logs::Logger::Log(LOG_NOTICE, "Error backuping devices state : « " + string(e.what()) + " »");
	}

	devices.clear();
}

void Devices::Delete(int id)
{
	unique_lock<mutex> llock(mutex_w);

	auto it = devices.find(id);
	if(it==devices.end())
		throw runtime_error("Unable to delete unknown device : " + to_string(id));

	auto device = it->second;

	// Flag device for removal once last shared pointed is released
	device->Delete();

	// Then erase it from our inventory
	devices.erase(device->GetID());

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
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

shared_ptr<Device> Devices::get_by_id(int id)
{
	auto it = devices.find(id);
	if(it==devices.end())
		throw out_of_range("Unknown device id : " + to_string(id));

	return it->second;
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

}

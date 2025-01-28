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

#include <device/DevicesOnOffImpl.hpp>
#include <device/DeviceTimeRange.hpp>
#include <device/DeviceHeater.hpp>
#include <device/DeviceCMV.hpp>
#include <device/DeviceHWS.hpp>
#include <configuration/Json.hpp>
#include <database/DB.hpp>
#include <logs/Logger.hpp>
#include <websocket/SolarControl.hpp>

#include <stdexcept>

using namespace std;

namespace device {

bool DevicesPtrComparator::operator()(DeviceOnOff *a, DeviceOnOff *b) const
{
	return a->GetPrio() < b->GetPrio();
}

DevicesOnOffImpl * DevicesOnOffImpl::instance = 0;

DevicesOnOffImpl::DevicesOnOffImpl()
{
	reload();

	instance = this;
}

DeviceOnOff *DevicesOnOffImpl::get_by_id(unsigned int id) const
{
	auto it = id_device.find(id);
	if(it==id_device.end())
		throw invalid_argument("Unknown OnOff device ID « " + to_string(id) + " »");

	return it->second;
}

void DevicesOnOffImpl::reload(bool notify)
{
	logs::Logger::Log(LOG_NOTICE, "Loading devices OnOff");

	free();

	database::DB db;

	auto res = db.Query("SELECT device_id, device_name, device_type, device_config FROM t_device WHERE device_type IN('timerange', 'heater', 'hws', 'cmv')"_sql);
	while(res.FetchRow())
	{
		configuration::Json config((string)res["device_config"]);

		DeviceOnOff *device;
		if((string)res["device_type"]=="timerange")
			device = new DeviceTimeRange(res["device_id"], res["device_name"], config);
		else if((string)res["device_type"]=="heater")
			device = new DeviceHeater(res["device_id"], res["device_name"], config);
		else if((string)res["device_type"]=="cmv")
			device = new DeviceCMV(res["device_id"], res["device_name"], config);
		else if((string)res["device_type"]=="hws")
		{
			device = new DeviceHWS(res["device_id"], res["device_name"], config);
			hws_id = device->GetID();
		}
		else
			throw invalid_argument("Invalid device type « " + string(res["device_type"]) + " »");

		insert(device);
		id_device.insert(pair<unsigned int, DeviceOnOff *>(res["device_id"], device));
	}

	if(notify && websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
}

void DevicesOnOffImpl::unload()
{
	free();
}

void DevicesOnOffImpl::free()
{
	for(auto it = begin(); it!=end(); ++it)
	{
		Device *device = *it;
		delete device;
	}

	clear();
	id_device.clear();
}

DevicesOnOffImpl::~DevicesOnOffImpl()
{
	free();
}

}

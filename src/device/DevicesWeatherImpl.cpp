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

#include <device/DevicesWeatherImpl.hpp>
#include <device/DeviceHTWifi.hpp>
#include <device/DeviceHTBluetooth.hpp>
#include <device/DeviceWind.hpp>
#include <configuration/Json.hpp>
#include <database/DB.hpp>
#include <logs/Logger.hpp>
#include <websocket/SolarControl.hpp>
#include <nlohmann/json.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;

namespace device {

DevicesWeatherImpl * DevicesWeatherImpl::instance = 0;

DevicesWeatherImpl::DevicesWeatherImpl()
{
	reload();

	instance = this;
}

DeviceWeather *DevicesWeatherImpl::get_by_id(unsigned int id) const
{
	auto it = id_device.find(id);
	if(it==id_device.end())
		throw invalid_argument("Unknown Weather device ID « " + to_string(id) + " »");

	return it->second;
}

void DevicesWeatherImpl::reload(bool notify)
{
	logs::Logger::Log(LOG_NOTICE, "Loading devices Weather");

	free();

	database::DB db;

	auto res = db.Query("SELECT device_id, device_name, device_type, device_config FROM t_device WHERE device_type IN('ht', 'htmini', 'wind')"_sql);
	while(res.FetchRow())
	{
		configuration::Json config((string)res["device_config"]);

		DeviceWeather *device;
		if((string)res["device_type"]=="ht")
			device = new DeviceHTWifi(res["device_id"], res["device_name"], config);
		else if((string)res["device_type"]=="htmini")
			device = new DeviceHTBluetooth(res["device_id"], res["device_name"], config);
		else if((string)res["device_type"]=="wind")
			device = new DeviceWind(res["device_id"], res["device_name"], config);
		else
			throw invalid_argument("Invalid device type « " + string(res["device_type"]) + " »");

		insert(device);
		id_device.insert(pair<unsigned int, DeviceWeather *>(res["device_id"], device));
	}

	if(notify && websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
}

void DevicesWeatherImpl::unload()
{
	free();
}

void DevicesWeatherImpl::free()
{
	for(auto it = begin(); it!=end(); ++it)
	{
		Device *device = *it;
		delete device;
	}

	clear();
	id_device.clear();
}

DevicesWeatherImpl::~DevicesWeatherImpl()
{
	free();
}

}


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

#include <api/Device.hpp>
#include <database/DB.hpp>
#include <configuration/Json.hpp>
#include <device/Devices.hpp>
#include <device/electrical/DeviceOnOff.hpp>
#include <nlohmann/json.hpp>

#include <stdexcept>

using namespace std;
using database::DB;
using namespace device;
using nlohmann::json;

namespace api
{

int Device::insert_device(const std::string &type, const std::string &name, const configuration::Json &config)
{
	if(name=="")
		throw invalid_argument("Name cannot be empty");

	DB db;

	db.Query(
		"INSERT INTO t_device (device_type, device_name, device_config) VALUES(%s, %s, %s)"_sql
		<<type<<name<<config.ToString()
	);

	int id = (int)db.InsertID(); // Hope we don't have more then 2^32 devices ;)

	Devices devices;
	devices.Load(id, name, type, config);

	return id;
}

void Device::update_device(int id, const std::string &name, const configuration::Json &config)
{
	if(name=="")
		throw invalid_argument("Name cannot be empty");

	DB db;

	db.Query(
		"UPDATE t_device SET device_name=%s, device_config=%s WHERE device_id=%i"_sql
		<<name<<config.ToString()<<id
	);
}

void Device::update_prio(int id, int new_prio)
{
	auto device = Devices::GetByID<DeviceElectrical>(id);
	if(device->GetCategory()!=ONOFF)
		throw invalid_argument("Device has no priority");

	auto onoff = dynamic_pointer_cast<DeviceOnOff>(device);

	DB db;

	json j_config = onoff->GetConfig();
	j_config["prio"] = new_prio;
	db.Query(
		"UPDATE t_device SET device_config=%s WHERE device_id=%i"_sql
		<<j_config.dump()<<onoff->GetID()
	);
}

}




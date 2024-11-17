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

#include <api/DeviceHT.hpp>
#include <database/DB.hpp>
#include <device/DevicesHT.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using database::DB;

namespace api
{

void DeviceHT::check_config(const configuration::Json &j_config, const string &device_type)
{
	if(device_type=="ht")
		j_config.Check("mqtt_id", "string");

	if(device_type=="htmini")
		j_config.Check("ble_addr", "string");
}

json DeviceHT::HandleMessage(const string &cmd, const configuration::Json &j_params)
{
	json j_res;
	DB db;

	device::DevicesHT devices;

	if(cmd=="set")
	{
		int device_id =j_params.GetInt("device_id");
		string device_name = j_params.GetString("device_name");
		auto device_config = j_params.GetObject("device_config");

		string device_type = devices.GetByID(device_id)->GetType();

		check_config(device_config, device_type);

		db.Query(
			"UPDATE t_device SET device_name=%s, device_config=%s WHERE device_id=%i"_sql
			<<device_name<<device_config.ToString()<<device_id
		);

		devices.Reload();

		return json();
	}
	else if(cmd=="create")
	{
		string device_name = j_params.GetString("device_name");
		string device_type = j_params.GetString("device_type");

		if(device_type!="ht" && device_type!="htmini")
			throw invalid_argument("Invalid device type : « " + device_type + " »");

		auto j_config = j_params.GetObject("device_config");
		check_config(j_config, device_type);

		string device_config = j_config.ToString();

		db.Query(
			"INSERT INTO t_device (device_type, device_name, device_config) VALUES(%s, %s, %s)"_sql
			<<device_type<<device_name<<device_config
		);

		devices.Reload();

		return json();
	}
	else if(cmd=="delete")
	{
		int device_id = j_params.GetInt("device_id");

		db.Query("DELETE FROM t_device WHERE device_id=%i"_sql<<device_id);

		devices.Reload();

		return json();
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « deviceht »");
}

}


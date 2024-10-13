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
#include <device/Devices.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using database::DB;

namespace api
{

json Device::HandleMessage(const string &cmd, const nlohmann::json &j_params)
{
	json j_res;
	DB db;

	if(cmd=="list")
	{
		auto res = db.Query("SELECT device_id, device_name, device_prio FROM t_device"_sql);
		j_res = json::array();
		while(res.FetchRow())
		{
			json device;
			device["device_id"] = res["device_id"];
			device["device_name"] = res["device_name"];
			device["device_prio"] = res["device_prio"];

			j_res.push_back(device);
		}

		return j_res;
	}
	else if(cmd=="get")
	{
		if(!j_params.contains("device_id"))
			throw invalid_argument("Missing device_id");

		auto res = db.Query("SELECT device_id, device_name, device_prio, device_config FROM t_device WHERE device_id = %i"_sql <<(int)j_params["device_id"]);
		if(!res.FetchRow())
			throw invalid_argument("Uknown device_id : « " + string(j_params["device_id"]) + " »");

		json device;
		device["device_id"] = res["device_id"];
		device["device_name"] = res["device_name"];
		device["device_prio"] = res["device_prio"];
		device["device_config"] = json::parse(string(res["device_config"]));

		return device;
	}
	else if(cmd=="set")
	{
		int device_id =j_params["device_id"];
		string device_name = j_params["device_name"];
		int device_prio = j_params["device_prio"];
		string device_config = j_params["device_config"].dump();

		db.Query(
			"UPDATE t_device SET device_name=%s, device_prio=%i, device_config=%s WHERE device_id=%i"_sql
			<<device_name<<device_prio<<device_config<<device_id
		);

		device::Devices::GetInstance()->Reload();

		return json();
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « device »");
}

}

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

#include <api/DeviceOnOff.hpp>
#include <database/DB.hpp>
#include <device/Devices.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using database::DB;
using device::Devices;
using device::DevicesOnOff;

namespace api
{

void DeviceOnOff::check_config(const nlohmann::json &j_config, const string &device_type)
{
	check_param(j_config, "prio", "int");
	check_param(j_config, "ip", "string");
	check_param(j_config, "expected_consumption", "int");
	check_param(j_config, "offload", "array");
	check_param(j_config, "force", "array");
	check_param(j_config, "remainder", "array");
	check_param(j_config, "min_on_time", "int");
	check_param(j_config, "min_on_for_last", "int");
	check_param(j_config, "min_on", "int");
	check_param(j_config, "min_off", "int");

	if(device_type=="heater")
	{
		check_param(j_config, "ht_device_id", "int");
		check_param(j_config, "force_max_temperature", "float");
		check_param(j_config, "offload_max_temperature", "float");
	}
}

json DeviceOnOff::HandleMessage(const string &cmd, const nlohmann::json &j_params)
{
	json j_res;
	DB db;

	DevicesOnOff &devices = Devices::GetInstance()->GetOnOff();

	if(cmd=="list")
	{
		devices.Lock();

		j_res = json::array();
		for(auto device : devices)
		{
			json j_device;
			j_device["device_id"] = device->GetID();
			j_device["device_name"] = device->GetName();
			j_device["state"] = device->GetState();
			j_device["manual"] = device->IsManual();

			j_res.push_back(j_device);
		}

		devices.Unlock();

		return j_res;
	}
	else if(cmd=="get")
	{
		if(!j_params.contains("device_id"))
			throw invalid_argument("Missing device_id");

		auto res = db.Query("SELECT device_id, device_name, device_type, device_config FROM t_device WHERE device_id = %i"_sql <<(int)j_params["device_id"]);
		if(!res.FetchRow())
			throw invalid_argument("Uknown device_id : « " + string(j_params["device_id"]) + " »");

		json device;
		device["device_id"] = res["device_id"];
		device["device_name"] = res["device_name"];
		device["device_type"] = res["device_type"];
		device["device_config"] = json::parse(string(res["device_config"]));

		return device;
	}
	else if(cmd=="set")
	{
		check_param(j_params, "device_id", "int");
		check_param(j_params, "device_name", "string");
		check_param(j_params, "device_config", "object");

		int device_id =j_params["device_id"];
		string device_name = j_params["device_name"];
		string device_config = j_params["device_config"].dump();

		string device_type = devices.GetByID(device_id)->GetType();

		auto j_config = j_params["device_config"];
		check_config(j_config, device_type);

		db.Query(
			"UPDATE t_device SET device_name=%s, device_config=%s WHERE device_id=%i"_sql
			<<device_name<<device_config<<device_id
		);

		devices.Reload();

		return json();
	}
	else if(cmd=="create")
	{
		check_param(j_params, "device_name", "string");
		check_param(j_params, "device_type", "string");
		check_param(j_params, "device_config", "object");

		string device_name = j_params["device_name"];
		string device_type = j_params["device_type"];

		if(device_type!="timerange-plug" && device_type!="heater")
			throw invalid_argument("Invalid device type : « " + device_type + " »");

		auto j_config = j_params["device_config"];
		check_config(j_config, device_type);

		string device_config = j_params["device_config"].dump();

		db.Query(
			"INSERT INTO t_device (device_type, device_name, device_config) VALUES(%s, %s, %s)"_sql
			<<device_type<<device_name<<device_config
		);

		devices.Reload();

		return json();
	}
	else if(cmd=="delete")
	{
		check_param(j_params, "device_id", "int");

		int device_id = j_params["device_id"];

		db.Query("DELETE FROM t_device WHERE device_id=%i"_sql<<device_id);

		devices.Reload();

		return json();
	}
	else if(cmd=="setstate")
	{
		check_param(j_params, "device_id", "int");
		check_param(j_params, "state", "string");

		int device_id =j_params["device_id"];
		string state = j_params["state"];
		if(state!="on" && state!="off" && state!="auto")
			throw invalid_argument("Invalid state : « " + state + " »");

		auto device = devices.GetByID(device_id);
		if(state=="auto")
			device->SetAutoState();
		else
			device->SetManualState(state=="on"?true:false);

		return json();
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « deviceonoff »");
}

}

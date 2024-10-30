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
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using database::DB;
using device::Devices;
using device::DevicesOnOff;

namespace api
{

void DeviceOnOff::check_config(const configuration::Json &j_config, const string &device_type)
{
	j_config.Check("prio", "int");
	j_config.Check("control", "object");
	j_config.Check("expected_consumption", "int");
	j_config.Check("offload", "array");
	j_config.Check("force", "array");
	j_config.Check("remainder", "array");
	j_config.Check("min_on_time", "int");
	j_config.Check("min_on_for_last", "int");
	j_config.Check("min_on", "int");
	j_config.Check("min_off", "int");

	if(device_type=="heater")
	{
		j_config.Check("ht_device_id", "int");
		j_config.Check("force_max_temperature", "float");
		j_config.Check("offload_max_temperature", "float");
	}

	if(device_type=="hws")
	{
		j_config.Check("min_energy", "float");
		j_config.Check("min_energy_for_last", "int");
	}
}

json DeviceOnOff::HandleMessage(const string &cmd, const configuration::Json &j_params)
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
			if(device->GetType()=="hws")
				continue; // Ignore HWS special device

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
	else if(cmd=="get" || cmd=="gethws")
	{
		database::Query query;
		int device_id = 0;
		if(cmd=="get")
		{
			device_id = j_params.GetInt("device_id");

			query = "SELECT device_id, device_name, device_type, device_config FROM t_device WHERE device_id = %i"_sql <<device_id;
		}
		else
			query = "SELECT device_id, device_name, device_type, device_config FROM t_device WHERE device_type = 'hws'"_sql;

		auto res = db.Query(query);
		if(!res.FetchRow())
			throw invalid_argument("Uknown device_id : « " + to_string(device_id) + " »");

		json device;
		device["device_id"] = res["device_id"];
		device["device_name"] = res["device_name"];
		device["device_type"] = res["device_type"];
		device["device_config"] = json::parse(string(res["device_config"]));

		return device;
	}
	else if(cmd=="set")
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

		if(device_type!="timerange-plug" && device_type!="heater")
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
	else if(cmd=="setstate")
	{
		int device_id =j_params.GetInt("device_id");
		string state = j_params.GetString("state");
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

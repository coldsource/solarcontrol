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
#include <configuration/Json.hpp>
#include <device/DevicesOnOff.hpp>
#include <device/DeviceOnOff.hpp>
#include <websocket/SolarControl.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using device::DevicesOnOff;

namespace api
{

void DeviceOnOff::check_config(const configuration::Json &j_config, const string &device_type)
{
	j_config.Check("prio", "int");
	j_config.Check("control", "object");
	j_config.Check("expected_consumption", "int", false);
	j_config.Check("offload", "array", false);
	j_config.Check("force", "array", false);
	j_config.Check("remainder", "array", false);
	j_config.Check("min_on_time", "int", false);
	j_config.Check("min_on_for_last", "int", false);
	j_config.Check("min_on", "int", false);
	j_config.Check("min_off", "int", false);

	if(device_type=="heater")
	{
		j_config.Check("ht_device_id", "int");
		j_config.Check("force_max_temperature", "float");
		j_config.Check("offload_max_temperature", "float");
	}

	if(device_type=="cmv")
	{
		j_config.Check("ht_device_ids", "array");
		j_config.Check("force_max_moisture", "float");
		j_config.Check("offload_max_moisture", "float");
	}

	if(device_type=="hws")
	{
		j_config.Check("min_energy", "float");
		j_config.Check("min_energy_for_last", "int");
	}

	check_config_control(j_config.GetObject("control"));
}

void DeviceOnOff::check_config_control(const configuration::Json &j_config)
{
	j_config.Check("type", "string");

	string control_type = j_config.GetString("type");
	if(control_type!="plug" && control_type!="pro")
		throw invalid_argument("Invalid control type : « " + control_type + " »");

	j_config.Check("ip", "string");

	if(control_type=="pro")
		j_config.Check("outlet", "int");
}

json DeviceOnOff::HandleMessage(const string &cmd, const configuration::Json &j_params)
{
	json j_res;

	DevicesOnOff devices;

	if(cmd=="set")
	{
		int device_id =j_params.GetInt("device_id");
		string device_name = j_params.GetString("device_name");
		auto device_config = j_params.GetObject("device_config");

		string device_type = devices.GetByID(device_id)->GetType();

		check_config(device_config, device_type);

		update_device(device_id, device_name, device_config);

		devices.Reload();

		return json();
	}
	else if(cmd=="create")
	{
		string device_name = j_params.GetString("device_name");
		string device_type = j_params.GetString("device_type");

		if(device_type!="timerange" && device_type!="heater" && device_type!="cmv")
			throw invalid_argument("Invalid device type : « " + device_type + " »");

		auto j_config = j_params.GetObject("device_config");
		check_config(j_config, device_type);

		insert_device(device_type, device_name, j_config);

		devices.Reload();

		return json();
	}
	else if(cmd=="delete")
	{
		int device_id = j_params.GetInt("device_id");

		delete_device(device_id);

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

		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);

		return json();
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « deviceonoff »");
}

}

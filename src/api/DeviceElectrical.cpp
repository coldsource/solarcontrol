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

#include <api/DeviceElectrical.hpp>
#include <configuration/Json.hpp>
#include <device/Devices.hpp>
#include <device/DeviceElectrical.hpp>
#include <websocket/SolarControl.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using device::Devices;

namespace api
{

void DeviceElectrical::check_config(const configuration::Json &j_config, const string &device_type)
{
	if(device_type!="passive" && device_type!="grid" && device_type!="pv" && device_type!="hws")
		j_config.Check("prio", "int"); // Prio is mandatory for all onoff devices
	else
		j_config.Check("meter", "object"); // Meter is mandatory for passive devices

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

		int ht_device_id = j_config.GetInt("ht_device_id");
		try
		{
			Devices devices;
			devices.GetWeatherByID(ht_device_id);
		}
		catch(exception &e)
		{
			throw invalid_argument("Associated thermometer is mandatory");
		}
	}

	if(device_type=="cmv")
	{
		j_config.Check("ht_device_ids", "array");
		j_config.Check("force_max_moisture", "float");
		j_config.Check("offload_max_moisture", "float");

		if(j_config.GetArray("ht_device_ids").size()==0)
			throw invalid_argument("Associated hygrometer is mandatory");

		try
		{
			Devices devices;
			for(auto device_id : j_config.GetArray("ht_device_ids"))
				devices.GetWeatherByID((int)device_id);
		}
		catch(exception &e)
		{
			throw invalid_argument("Associated hygrometer is mandatory");
		}
	}

	if(device_type=="hws")
	{
		j_config.Check("min_energy", "float");
		j_config.Check("min_energy_for_last", "int");
	}

	if(j_config.Has("control"))
	{
		j_config.Check("control", "object");
		check_config_control(j_config.GetObject("control"));
	}

	if(j_config.Has("meter"))
	{
		j_config.Check("meter", "object");
		check_config_meter(j_config.GetObject("meter"));
	}
}

void DeviceElectrical::check_config_control(const configuration::Json &j_config)
{
	j_config.Check("type", "string");

	string control_type = j_config.GetString("type");
	if(control_type!="plug" && control_type!="pro")
		throw invalid_argument("Invalid control type : « " + control_type + " »");

	j_config.Check("ip", "string");

	if(control_type=="pro")
		j_config.Check("outlet", "int");
}

void DeviceElectrical::check_config_meter(const configuration::Json &j_config)
{
	j_config.Check("type", "string");

	string control_type = j_config.GetString("type");
	if(control_type!="3em")
		throw invalid_argument("Invalid control type : « " + control_type + " »");

	j_config.Check("mqtt_id", "string");
	if(j_config.GetString("mqtt_id")=="")
		throw invalid_argument("MQTT ID is required");

	if(control_type=="3em")
		j_config.Check("phase", "string");
}

json DeviceElectrical::HandleMessage(const string &cmd, const configuration::Json &j_params)
{
	json j_res;

	Devices devices;

	if(cmd=="set")
	{
		int device_id =j_params.GetInt("device_id");
		string device_name = j_params.GetString("device_name");
		auto device_config = j_params.GetObject("device_config");

		string device_type = devices.GetElectricalByID(device_id)->GetType();

		check_config(device_config, device_type);

		update_device(device_id, device_name, device_config);

		devices.Reload(device_id);

		return json();
	}
	else if(cmd=="create")
	{
		string device_name = j_params.GetString("device_name");
		string device_type = j_params.GetString("device_type");

		if(device_type!="timerange" && device_type!="heater" && device_type!="cmv" && device_type!="passive")
			throw invalid_argument("Invalid device type : « " + device_type + " »");

		auto j_config = j_params.GetObject("device_config");
		check_config(j_config, device_type);

		int device_id = insert_device(device_type, device_name, j_config);

		devices.Reload(device_id);

		return json();
	}
	else if(cmd=="delete")
	{
		int device_id = j_params.GetInt("device_id");

		delete_device(device_id);

		devices.Reload(device_id);

		return json();
	}
	else if(cmd=="setstate")
	{
		int device_id =j_params.GetInt("device_id");
		string state = j_params.GetString("state");
		if(state!="on" && state!="off" && state!="auto")
			throw invalid_argument("Invalid state : « " + state + " »");

		auto device = devices.GetElectricalByID(device_id);

		if(device->GetType()=="passive")
			throw invalid_argument("Could not change state on passive device");

		if(state=="auto")
			device->SetAutoState();
		else
			device->SetManualState(state=="on"?true:false);

		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);

		return json();
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « deviceelectrical »");
}

}

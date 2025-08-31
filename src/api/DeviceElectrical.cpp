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
#include <device/DeviceFactory.hpp>
#include <device/electrical/DeviceElectrical.hpp>
#include <device/electrical/DeviceOnOff.hpp>
#include <websocket/SolarControl.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using device::Devices;

namespace api
{

json DeviceElectrical::HandleMessage(const string &cmd, const configuration::Json &j_params)
{
	json j_res;

	Devices devices;

	if(cmd=="set")
	{
		int device_id =j_params.GetInt("device_id");
		string device_name = j_params.GetString("device_name");
		string device_type = devices.GetElectricalByID(device_id)->GetType();
		auto device_config = j_params.GetObject("device_config");

		device::DeviceFactory::CheckConfig(device_type, device_config);

		update_device(device_id, device_name, device_config);

		devices.Reload(device_id);

		return json();
	}
	else if(cmd=="setprio")
	{
		json id_prio = j_params;
		for(auto it : id_prio.items())
		{
			int device_id = stoi(it.key());
			int prio = it.value();

			update_prio(device_id, prio);
		}

		devices.Reload();

		return json();
	}
	else if(cmd=="create")
	{
		string device_name = j_params.GetString("device_name");
		string device_type = j_params.GetString("device_type");

		auto device_config = j_params.GetObject("device_config");
		device::DeviceFactory::CheckConfig(device_type, device_config);

		insert_device(device_type, device_name, device_config);

		return json();
	}
	else if(cmd=="delete")
	{
		int device_id = j_params.GetInt("device_id");

		devices.Delete(device_id);

		return json();
	}
	else if(cmd=="setstate")
	{
		int device_id =j_params.GetInt("device_id");
		string state = j_params.GetString("state");
		if(state!="on" && state!="off" && state!="auto")
			throw invalid_argument("Invalid state : « " + state + " »");

		auto device = devices.GetElectricalByID(device_id);

		if(device->GetCategory()!=device::ONOFF)
			throw invalid_argument("Could not change state on passive device");

		auto device_onoff = dynamic_pointer_cast<device::DeviceOnOff>(device);

		if(state=="auto")
			device_onoff->SetAutoState();
		else
			device_onoff->SetManualState(state=="on"?true:false);

		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);

		return json();
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « deviceelectrical »");
}

}

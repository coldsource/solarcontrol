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
#include <device/Devices.hpp>
#include <device/DeviceFactory.hpp>
#include <device/weather/DeviceWeather.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;

namespace api
{

json DeviceHT::HandleMessage(const string &cmd, const configuration::Json &j_params)
{
	json j_res;

	device::Devices devices;

	if(cmd=="set")
	{
		int device_id =j_params.GetInt("device_id");
		string device_name = j_params.GetString("device_name");
		auto device_config = j_params.GetObject("device_config");

		string device_type = devices.GetWeatherByID(device_id)->GetType();

		device::DeviceFactory::CheckConfig(device_type, device_config);

		update_device(device_id, device_name, device_config);

		devices.Reload(device_id);

		return json();
	}
	else if(cmd=="create")
	{
		string device_name = j_params.GetString("device_name");
		string device_type = j_params.GetString("device_type");

		auto device_config = j_params.GetObject("device_config");
		device::DeviceFactory::CheckConfig(device_type, device_config);

		int device_id = insert_device(device_type, device_name, device_config);

		devices.Reload(device_id);

		return json();
	}
	else if(cmd=="delete")
	{
		int device_id = j_params.GetInt("device_id");

		auto dependent_device = devices.IsInUse(device_id);
		if(dependent_device)
			throw runtime_error("Device is in use by « " + dependent_device->GetName() + " »");

		delete_device(device_id);

		devices.Reload(device_id);

		return json();
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « deviceht »");
}

}


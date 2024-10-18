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
#include <device/Devices.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using database::DB;

namespace api
{

json DeviceHT::HandleMessage(const string &cmd, const nlohmann::json &j_params)
{
	json j_res;
	DB db;

	if(cmd=="list")
	{
		device::DevicesHT &devices = device::Devices::GetInstance()->GetHT();

		devices.Lock();

		j_res = json::array();
		for(auto device : devices)
		{
			json j_device;
			j_device["device_id"] = device->GetID();
			j_device["device_name"] = device->GetName();
			j_device["temperature"] = device->GetTemperature();
			j_device["humidity"] = device->GetHumidity();

			j_res.push_back(j_device);
		}

		devices.Unlock();

		return j_res;
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « deviceht »");
}

}


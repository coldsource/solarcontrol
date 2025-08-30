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

#include <device/weather/DeviceWeather.hpp>

using namespace std;
using nlohmann::json;

namespace device {

json DeviceWeather::ToJson() const
{
	unique_lock<recursive_mutex> llock(lock);

	json j_device;

	j_device["device_id"] = GetID();
	j_device["device_type"] = GetType();
	j_device["device_name"] = GetName();
	j_device["device_config"] = (json)GetConfig();
	j_device["temperature"] = GetTemperature();
	j_device["humidity"] = GetHumidity();
	j_device["wind"] = GetWind();

	return j_device;
}

}

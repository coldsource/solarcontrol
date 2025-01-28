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

#include <device/DeviceHeater.hpp>
#include <configuration/Json.hpp>
#include <device/DevicesWeather.hpp>
#include <device/DeviceWeather.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

DeviceHeater::DeviceHeater(unsigned int id, const string &name, const configuration::Json &config):DeviceTimeRange(id, name, config)
{
	ht_device_id = config.GetInt("ht_device_id");
	force_max_temperature = config.GetFloat("force_max_temperature");
	offload_max_temperature = config.GetFloat("offload_max_temperature");
}

en_wanted_state DeviceHeater::GetWantedState() const
{
	DevicesWeather devices;
	auto ht = devices.GetByID(ht_device_id);

	en_wanted_state wanted_state = DeviceTimeRange::GetWantedState();
	if(wanted_state==UNCHANGED)
		return UNCHANGED;

	if(wanted_state==ON)
		return (ht->GetTemperature()<force_max_temperature)?ON:OFF;

	if(wanted_state==OFFLOAD)
		return (ht->GetTemperature()<offload_max_temperature)?OFFLOAD:OFF;

	return OFF;
}

}

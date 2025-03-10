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

#include <device/electrical/DeviceHeater.hpp>
#include <device/weather/DeviceWeather.hpp>
#include <configuration/Json.hpp>
#include <device/Devices.hpp>
#include <logs/Logger.hpp>

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

void DeviceHeater::CheckConfig(const configuration::Json &conf)
{
	DeviceTimeRange::CheckConfig(conf);

	conf.Check("ht_device_id", "int");
	conf.Check("force_max_temperature", "float");
	conf.Check("offload_max_temperature", "float");

	int ht_device_id = conf.GetInt("ht_device_id");
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

en_wanted_state DeviceHeater::GetWantedState() const
{
	Devices devices;
	DeviceWeather *ht;
	try
	{
		ht = devices.GetWeatherByID(ht_device_id);
	}
	catch(exception &e)
	{
		// Associated thermometer has been removed, fallback to safe mode
		logs::Logger::Log(LOG_NOTICE, "Missing thermometer for device « " + GetName() + " », forcing device to off");
		return OFF;
	}

	en_wanted_state wanted_state = DeviceTimeRange::GetWantedState();
	if(wanted_state==UNCHANGED)
		return UNCHANGED;

	if(wanted_state==ON)
		return (ht->GetTemperature()<force_max_temperature)?ON:OFF;

	if(wanted_state==OFFLOAD)
		return (ht->GetTemperature()<offload_max_temperature)?OFFLOAD:OFF;

	return OFF;
}

bool DeviceHeater::Depends(int device_id) const
{
	return device_id==ht_device_id;
}

}

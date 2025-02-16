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

#include <device/DeviceCMV.hpp>
#include <configuration/Json.hpp>
#include <datetime/Timestamp.hpp>
#include <device/Devices.hpp>
#include <device/DeviceWeather.hpp>
#include <logs/Logger.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

DeviceCMV::DeviceCMV(unsigned int id, const string &name, const configuration::Json &config):DeviceTimeRange(id, name, config)
{
	for(auto device_id : config.GetArray("ht_device_ids"))
		ht_device_ids.insert((unsigned int)device_id);

	force_max_moisture = config.GetFloat("force_max_moisture");
	offload_max_moisture = config.GetFloat("offload_max_moisture");
	max_on = config.GetFloat("max_on");
}

en_wanted_state DeviceCMV::GetWantedState() const
{
	Devices devices;

	double max_moisture = 0;
	try
	{
		for(auto ht_device_id : ht_device_ids)
		{
			auto ht = devices.GetWeatherByID(ht_device_id);
			if(ht->GetHumidity()>max_moisture)
				max_moisture = ht->GetHumidity();
		}
	}
	catch(exception &e)
	{
		// Associated hygrometer has been removed, fallback to safe mode
		logs::Logger::Log(LOG_NOTICE, "Missing hygrometer for device « " + GetName() + " », forcing device to off");
		return OFF;
	}

	en_wanted_state wanted_state = DeviceTimeRange::GetWantedState();
	if(wanted_state==UNCHANGED)
		return UNCHANGED;

	if(wanted_state==ON)
		return (max_moisture>force_max_moisture)?ON:OFF;

	if(wanted_state==OFFLOAD)
		return (max_moisture>offload_max_moisture)?OFFLOAD:OFF;

	return OFF;
}

bool DeviceCMV::Depends(int device_id) const
{
	for(auto ht_device_id : ht_device_ids)
		if(ht_device_id==device_id)
			return true;
	return false;
}

}


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

#include <device/electrical/DeviceCMV.hpp>
#include <device/weather/DeviceWeather.hpp>
#include <configuration/Json.hpp>
#include <datetime/Timestamp.hpp>
#include <device/Devices.hpp>
#include <logs/Logger.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

DeviceCMV::DeviceCMV(int id):DeviceTimeRange(id)
{
}

void DeviceCMV::CheckConfig(const configuration::Json &conf)
{
	DeviceTimeRange::CheckConfig(conf);

	conf.Check("ht_device_ids", "array");

	check_timeranges(conf, "offload");
	check_timeranges(conf, "force");

	if(conf.GetArray("ht_device_ids").size()==0)
		throw invalid_argument("Associated hygrometer is mandatory");

	try
	{
		Devices devices;
		for(auto device_id : conf.GetArray("ht_device_ids"))
			devices.GetWeatherByID((int)device_id);
	}
	catch(exception &e)
	{
		throw invalid_argument("Associated hygrometer is mandatory");
	}
}

void DeviceCMV::reload(const configuration::Json &config)
{
	DeviceTimeRange::reload(config);

	for(auto device_id : config.GetArray("ht_device_ids"))
		ht_device_ids.insert(device_id);

	max_on = config.GetUInt("max_on");
}

void DeviceCMV::check_timeranges(const configuration::Json &conf, const string &name)
{
	if(!conf.Has(name))
		return;

	auto timeranges = conf.GetArray(name);
	for(auto timerange : timeranges)
	{
		if(!timerange.Has("data"))
			throw invalid_argument("Time range moisture is mandatory");

		auto data = timerange.GetObject("data");
		data.Check("moisture", "float");
	}
}

en_wanted_state DeviceCMV::get_wanted_state(configuration::Json *data_ptr) const
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

	configuration::Json timerange_data;
	en_wanted_state wanted_state = DeviceTimeRange::get_wanted_state(&timerange_data);

	if(data_ptr)
		*data_ptr = timerange_data; // Forward timerange data to caller if requested

	if(wanted_state==UNCHANGED)
		return UNCHANGED;


	if(wanted_state==ON)
		return (max_moisture>timerange_data.GetFloat("moisture"))?ON:OFF;

	if(wanted_state==OFFLOAD)
		return (max_moisture>timerange_data.GetFloat("moisture"))?OFFLOAD:OFF;

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


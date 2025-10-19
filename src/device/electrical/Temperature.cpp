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

#include <device/electrical/Temperature.hpp>
#include <device/weather/Weather.hpp>
#include <configuration/Json.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <device/Devices.hpp>
#include <logs/Logger.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

Temperature::Temperature(int id):TimeRange(id)
{
	ObserveConfiguration("control");
}

Temperature::~Temperature()
{
}

void Temperature::ConfigurationChanged(const configuration::ConfigurationPart * config)
{
	unique_lock<recursive_mutex> llock(lock);

	if(config->GetType()=="control")
	{
		absence_temperature = config->GetDouble("control.absence.temperature");
		absence = config->GetBool("control.absence.enabled");
	}

	TimeRange::ConfigurationChanged(config);
}

void Temperature::CheckConfig(const configuration::Json &conf)
{
	TimeRange::CheckConfig(conf);

	conf.Check("ht_device_id", "int");

	check_timeranges(conf, "offload");
	check_timeranges(conf, "force");

	int ht_device_id = conf.GetInt("ht_device_id");
	try
	{
		Devices::GetByID<Weather>(ht_device_id); // Check device exists
	}
	catch(exception &e)
	{
		throw excpt::Config("Associated thermometer is mandatory", "ht_device_id");
	}
}

void Temperature::reload(const configuration::Json &config)
{

	TimeRange::reload(config);

	ht_device_id = config.GetInt("ht_device_id");
}

void Temperature::check_timeranges(const configuration::Json &conf, const string &name)
{
	if(!conf.Has(name))
		return;

	auto timeranges = conf.GetArray(name);
	for(auto timerange : timeranges)
	{
		if(!timerange.Has("data"))
			throw excpt::Config("Time range temperature is mandatory", name);

		auto data = timerange.GetObject("data");
		data.Check("temperature", "float");
	}
}

en_wanted_state Temperature::get_wanted_state(configuration::Json *data_ptr) const
{
	shared_ptr<Weather> ht;
	try
	{
		ht = Devices::GetByID<Weather>(ht_device_id);
	}
	catch(exception &e)
	{
		// Associated thermometer has been removed, fallback to safe mode
		logs::Logger::Log(LOG_NOTICE, "Missing thermometer for device « " + GetName() + " », forcing device to off");
		return OFF;
	}

	configuration::Json timerange_data;
	en_wanted_state wanted_state = TimeRange::get_wanted_state(&timerange_data);

	if(data_ptr)
		*data_ptr = timerange_data; // Forward timerange data to caller if requested

	if(wanted_state==UNCHANGED)
		return UNCHANGED;

	// Enable forced mode only if temparature is not reached. If temperature is reached, try if offload is wanted
	if(wanted_state==ON && temp_check_force(ht->GetTemperature(), timerange_data.GetFloat("temperature")))
		return ON;

	// Offload is requested (may overlap with forced mode)
	if(wanted_state==OFFLOAD || WantOffload(&timerange_data))
	{
		if(data_ptr)
			*data_ptr = timerange_data; // Forward timerange data to caller if requested
		return temp_check_offload(ht->GetTemperature(), timerange_data.GetFloat("temperature"))?OFFLOAD:OFF;
	}

	return OFF;
}

bool Temperature::Depends(int device_id) const
{
	unique_lock<recursive_mutex> llock(lock);

	return device_id==ht_device_id;
}

json Temperature::ToJson() const
{
	unique_lock<recursive_mutex> llock(lock);

	json j_device = TimeRange::ToJson();

	j_device["temperature"] = Devices::GetByID<Weather>(ht_device_id)->GetTemperature();

	return j_device;
}

}

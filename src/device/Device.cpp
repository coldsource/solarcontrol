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

#include <device/Device.hpp>
#include <database/DB.hpp>
#include <sensor/Sensor.hpp>
#include <excpt/Context.hpp>
#include <excpt/Shelly.hpp>
#include <logs/Logger.hpp>

using namespace std;
using database::DB;
using nlohmann::json;

namespace device {

Device::~Device()
{
	if(deleted)
	{
		// We are marked for destruction, we muse remove ourself from database
		DB db;

		db.Query("DELETE FROM t_device WHERE device_id=%i"_sql<<id);
		db.Query("DELETE FROM t_device_state WHERE device_id=%i"_sql<<id);
		db.Query("DELETE FROM t_log_energy_detail WHERE device_id=%i"_sql<<id);
		db.Query("DELETE FROM t_log_ht WHERE device_id=%i"_sql<<id);
		db.Query("DELETE FROM t_log_wind WHERE device_id=%i"_sql<<id);
		db.Query("DELETE FROM t_log_state WHERE device_id=%i"_sql<<id);
	}
}

void Device::Reload(const string &name, const configuration::Json &j_config, bool device_enabled)
{
	auto new_config = build_config(j_config);

	new_config->name = name;
	new_config->enabled = device_enabled;
	new_config->json_config = j_config;

	// Lock before preparing config
	lock_guard<recursive_mutex> llock(lock);
	excpt::Context ctx("device", "In device « " + name + " »", {{"device_id", GetID()}});
	new_config->Prepare(this);

	config.store(new_config); // Atomically switch configuration
}

void Device::Delete()
{
		deleted = true; // Device might be in use, flag for removal in destructor
}

const configuration::Json Device::GetConfig() const
{
	auto lconfig = config.load();
	return lconfig->json_config;
}

void Device::StateRestore(const configuration::Json &last_state)
{
	lock_guard<recursive_mutex> llock(lock);

	state_restore(last_state);
}

json Device::ToJson() const
{
	auto lconfig = config.load();

	json j_device;

	j_device["device_id"] = GetID();
	j_device["device_type"] = GetType();
	j_device["device_category"] = GetCategory();
	j_device["device_name"] = lconfig->name;
	j_device["device_config"] = (json)lconfig->json_config;
	j_device["offline"] = (bool)offline;
	j_device["enabled"] = (bool)lconfig->enabled;

	return j_device;
}

void Device::DispatchMQTTMessage(const mqtt::st_message &mqtt_message)
{
	lock_guard<recursive_mutex> llock(lock);

	auto [sensor, changed] = config.load()->sensors.dispatch_mqtt_message(mqtt_message);
	if(sensor && changed)
		sensor_changed(sensor.get());
}

void Device::SensorsForceUpdate()
{
	auto lconfig = config.load();

	try
	{
		// Check all sensors unlocked to avoid locking the device for too long
		excpt::Context ctx("device", "Getting input state for device « " + lconfig->name + " »");

		for(const auto &sensor : lconfig->sensors.get())
		{
			if(sensor.second->ForceUpdate())
			{
				offline = false;

				lock_guard<recursive_mutex> llock(lock);
				sensor_changed(sensor.second.get());
			}
		}
	}
	catch(excpt::Shelly &e)
	{
		offline = true;
		e.Log(LOG_WARNING);
	}
	catch(...)
	{
		offline = true;
		logs::Logger::Log(LOG_WARNING, "Unknown error on sensor for device « " + lconfig->name + " »");
	}
}

bool Device::Configuration::has_sensor(const std::string &category) const
{
	return sensors.has(category);
}

}

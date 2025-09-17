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

using namespace std;
using database::DB;

namespace device {

Device::~Device()
{
	if(deleted)
	{
		// We are marked for destruction, we muse remove ourselved from database
		DB db;

		db.Query("DELETE FROM t_device WHERE device_id=%i"_sql<<id);
		db.Query("DELETE FROM t_device_state WHERE device_id=%i"_sql<<id);
		db.Query("DELETE FROM t_log_energy_detail WHERE device_id=%i"_sql<<id);
		db.Query("DELETE FROM t_log_ht WHERE device_id=%i"_sql<<id);
		db.Query("DELETE FROM t_log_wind WHERE device_id=%i"_sql<<id);
		db.Query("DELETE FROM t_log_state WHERE device_id=%i"_sql<<id);
	}
}

void Device::Reload(const string &name, const configuration::Json &config)
{
	unique_lock<recursive_mutex> llock(lock);

	this->name = name;
	this->config = config;

	// Reinit sensors before reload
	sensors.clear();

	reload(config); // Call protected reload to let children reload
}

void Device::Delete()
{
	unique_lock<recursive_mutex> llock(lock);

	deleted = true; // Device might be in use, flag for removal in destructor
}

string Device::GetName() const
{
	unique_lock<recursive_mutex> llock(lock);

	return name;
}
const configuration::Json Device::GetConfig() const
{
	unique_lock<recursive_mutex> llock(lock);

	return config;
}

void Device::StateRestore(const configuration::Json &last_state)
{
	state_restore(last_state);
	state_restored = true;
}

void Device::add_sensor(shared_ptr<sensor::Sensor> sensor, const string &name)
{
	sensors.insert(name, this, sensor);
}

}

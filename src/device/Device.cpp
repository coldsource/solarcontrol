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

void Device::Reload(const std::string &name, const configuration::Json &config)
{
	unique_lock<recursive_mutex> llock(mutex);

	this->name = name;
	this->config = config;
}

void Device::state_backup(const configuration::Json &state)
{
	DB db;

	db.Query("REPLACE INTO t_device_state(device_id, device_state) VALUES(%i, %s)"_sql<<id<<state.ToString());
}

const configuration::Json Device::state_restore()
{
	DB db;

	auto res = db.Query("SELECT device_state FROM t_device_state WHERE device_id=%i"_sql<<id);
	if(res.FetchRow())
		return configuration::Json(string(res["device_state"]));
	return configuration::Json();
}

}

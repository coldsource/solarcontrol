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

#include <api/Device.hpp>
#include <database/DB.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;
using database::DB;

namespace api
{

void Device::insert_device(const std::string &type, const std::string &name, const configuration::Json &config)
{
	DB db;

	db.Query(
		"INSERT INTO t_device (device_type, device_name, device_config) VALUES(%s, %s, %s)"_sql
		<<type<<name<<config.ToString()
	);
}

void Device::update_device(unsigned int id, const std::string &name, const configuration::Json &config)
{
	DB db;

	db.Query(
		"UPDATE t_device SET device_name=%s, device_config=%s WHERE device_id=%i"_sql
		<<name<<config.ToString()<<id
	);
}

void Device::delete_device(unsigned int id)
{
	DB db;

	db.Query("DELETE FROM t_device WHERE device_id=%i"_sql<<id);
	db.Query("DELETE FROM  t_device_state WHERE device_id=%i"_sql<<id);
	db.Query("DELETE FROM  t_log_energy_detail WHERE device_id=%i"_sql<<id);
	db.Query("DELETE FROM  t_log_ht WHERE device_id=%i"_sql<<id);
	db.Query("DELETE FROM  t_log_wind WHERE device_id=%i"_sql<<id);
	db.Query("DELETE FROM  t_log_state WHERE device_id=%i"_sql<<id);
}

}




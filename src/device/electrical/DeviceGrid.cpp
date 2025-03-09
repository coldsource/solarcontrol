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

#include <device/electrical/DeviceGrid.hpp>
#include <device/Devices.hpp>
#include <configuration/Json.hpp>
#include <database/DB.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

DeviceGrid::DeviceGrid(unsigned int id, const string &name, const configuration::Json &config):DevicePassive(id, name, config)
{
	// Override default counter for storing excess
	consumption = energy::Counter(id, "consumption", "excess");
}

void DeviceGrid::CreateInDB()
{
	database::DB db;
	auto res = db.Query("SELECT device_id FROM t_device WHERE device_type='grid'"_sql);
	if(res.FetchRow())
		return; // Already in database

	json config;

	json meter;
	meter["type"] = "3em";
	meter["mqtt_id"] = "";
	meter["phase"] = "a";
	config["meter"] = meter;

	db.Query("INSERT INTO t_device(device_id, device_type, device_name, device_config) VALUES(%i, 'grid', 'grid', %s)"_sql<<DEVICE_ID_GRID<<config.dump());
}

}

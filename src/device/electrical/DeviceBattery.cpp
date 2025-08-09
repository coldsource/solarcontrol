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

#include <device/electrical/DeviceBattery.hpp>
#include <device/Devices.hpp>
#include <configuration/Json.hpp>
#include <energy/ConfigurationEnergy.hpp>
#include <meter/Pro3EM.hpp>
#include <database/DB.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

DeviceBattery::DeviceBattery(unsigned int id, const string &name, const configuration::Json &config):DevicePassive(id, name, config)
{
	// Override default counter for storing production
	consumption = energy::Counter(id, "production");
}

void DeviceBattery::CreateInDB()
{
	database::DB db;
	auto res = db.Query("SELECT device_id FROM t_device WHERE device_type='battery'"_sql);
	if(res.FetchRow())
		return; // Already in database

	json config;

	json meter;
	meter["type"] = "dummy";
	config["meter"] = meter;

	db.Query("INSERT INTO t_device(device_id, device_type, device_name, device_config) VALUES(%i, 'battery', 'battery', %s)"_sql<<DEVICE_ID_BATTERY<<config.dump());
}

}



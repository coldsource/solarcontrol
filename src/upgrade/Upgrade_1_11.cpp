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

#include <upgrade/Upgrade_1_11.hpp>
#include <database/DB.hpp>
#include <nlohmann/json.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;

namespace upgrade {

bool Upgrade_1_11::is_needed()
{
	string v;

	try
	{
		v = get_version();
		return false; // Upgrade already done
	}
	catch(exception &e)
	{
		return true;
	}
}

void Upgrade_1_11::run()
{
	database::DB db;
	db.Query("ALTER TABLE t_device MODIFY `device_type`VARCHAR(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL"_sql);
	db.Query("REPLACE INTO t_storage(storage_name, storage_value) VALUES(%s, %s)"_sql<<"version"<<"1.11");

	// mqtt_id is now mandatory for control section
	auto res = db.Query("SELECT device_id, device_config FROM t_device"_sql);
	while(res.FetchRow())
	{
		int device_id = res["device_id"];
		json j_config = json::parse((string)res["device_config"]);

		if(!j_config.contains("control"))
			continue;

		auto control = j_config["control"];
		if(control.contains("mqtt_id"))
			continue;

		j_config["control"]["mqtt_id"] = "default";
		string device_config = j_config.dump();
		db.Query("UPDATE t_device SET device_config = %s WHERE device_id = %i"_sql<<device_config<<device_id);
	}
}

}


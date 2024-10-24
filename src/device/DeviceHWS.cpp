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

#include <device/DeviceHWS.hpp>
#include <energy/GlobalMeter.hpp>
#include <database/DB.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

DeviceHWS::DeviceHWS(unsigned int id, const string &name, const json &config):DeviceTimeRange(id, name, config)
{
	check_config_parameters(config, {"min_energy", "min_energy_for_last"});

	min_energy = config["min_energy"];
	min_energy_for_last = config["min_energy_for_last"];
}

bool DeviceHWS::WantRemainder() const
{
	double last_energy = energy::GlobalMeter::GetInstance()->GetTotalHWSConsuptionForLast(min_energy_for_last);
	return remainder.IsActive() && last_energy<min_energy;
}

void DeviceHWS::CreateInDB()
{
	database::DB db;
	auto res = db.Query("SELECT device_id FROM t_device WHERE device_type='hws'"_sql);
	if(res.FetchRow())
		return; // Already in database

	string config = "{\"prio\": 0, \"ip\": \"192.168.16.21\", \"min_energy\": 0, \"min_energy_for_last\": 1, \"force\": [], \"offload\": [], \"remainder\": [], \"expected_consumption\": 0, \"min_on_time\": 0, \"min_off_time\": 0, \"min_on_for_last\": 0, \"min_on\": 0, \"min_off\": 0}";
	db.Query("INSERT INTO t_device(device_type, device_name, device_config) VALUES('hws', 'hws', %s)"_sql<<config);
}

}


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

#include <device/DevicesOnOff.hpp>
#include <device/DeviceTimeRange.hpp>
#include <database/DB.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace device {

DevicesOnOff * DevicesOnOff::instance = 0;

DevicesOnOff::DevicesOnOff()
{
	Reload();

	instance = this;
}

void DevicesOnOff::Reload()
{
	unique_lock<mutex> llock(d_mutex);

	printf("Loading devices OnOff\n");

	free();

	database::DB db;

	auto res = db.Query("SELECT device_id, device_name, device_type, device_config FROM t_device WHERE device_type IN('timerange-plug')"_sql);
	while(res.FetchRow())
	{
		auto config = json::parse((string)res["device_config"]);

		DeviceOnOff *device;
		if((string)res["device_type"]=="timerange-plug")
			device = new DeviceTimeRange(res["device_id"], res["device_name"], config);

		insert(device);
	}
}

void DevicesOnOff::free()
{
	for(auto it = begin(); it!=end(); ++it)
	{
		Device *device = *it;
		delete device;
	}

	clear();
}

DevicesOnOff::~DevicesOnOff()
{
	free();
}

}

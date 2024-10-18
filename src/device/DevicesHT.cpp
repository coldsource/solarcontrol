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

#include <device/DevicesHT.hpp>
#include <device/DeviceHT.hpp>
#include <database/DB.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace device {

DevicesHT * DevicesHT::instance = 0;

DevicesHT::DevicesHT()
{
	Reload();

	instance = this;
}

void DevicesHT::Reload()
{
	unique_lock<mutex> llock(d_mutex);

	printf("Loading devices HT\n");

	free();

	database::DB db;

	auto res = db.Query("SELECT device_id, device_name, device_type, device_config FROM t_device WHERE device_type IN('ht')"_sql);
	while(res.FetchRow())
	{
		auto config = json::parse((string)res["device_config"]);

		DeviceHT *device;
		if((string)res["device_type"]=="ht")
			device = new DeviceHT(res["device_id"], res["device_name"], config);

		insert(device);
	}
}

void DevicesHT::free()
{
	for(auto it = begin(); it!=end(); ++it)
	{
		Device *device = *it;
		delete device;
	}

	clear();
}

DevicesHT::~DevicesHT()
{
	free();
}

}


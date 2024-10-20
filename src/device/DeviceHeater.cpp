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

#include <device/DeviceHeater.hpp>
#include <datetime/Timestamp.hpp>
#include <device/DevicesHT.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

DeviceHeater::DeviceHeater(unsigned int id, const string &name, const json &config):DeviceTimeRange(id, name, config)
{
	check_config_parameters(config, {"ht_device_id", "force_max_temperature", "offload_max_temperature"});

	ht_device_id = config["ht_device_id"];
	force_max_temperature = config["force_max_temperature"];
	offload_max_temperature = config["offload_max_temperature"];
}

bool DeviceHeater::state_on_condition() const
{
	auto ht = DevicesHT::GetInstance()->GetByID(ht_device_id);

	if(IsForced())
		return (ht->GetTemperature()<force_max_temperature);

	if(WantOffload())
		return (ht->GetTemperature()<offload_max_temperature);

	return false;
}

}

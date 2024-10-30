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

#include <device/DeviceCMV.hpp>
#include <configuration/Json.hpp>
#include <datetime/Timestamp.hpp>
#include <device/DevicesHT.hpp>
#include <energy/GlobalMeter.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

DeviceCMV::DeviceCMV(unsigned int id, const string &name, const configuration::Json &config):DeviceTimeRange(id, name, config)
{
	for(auto device_id : config.GetArray("ht_device_ids"))
		ht_device_ids.insert((unsigned int)device_id);

	force_max_moisture = config.GetFloat("force_max_moisture");
	offload_max_moisture = config.GetFloat("offload_max_moisture");
	max_on = config.GetFloat("max_on");
}

// CMV Devices are based on Gross Available Power (other devices are based on Net)
bool DeviceCMV::WantOffload() const
{
	if(!offload.IsActive())
		return false;

	if(GetState())
		return (global_meter->GetGrossAvailablePower()>0); // We are already on, so stay on as long as we have power to offload

	return (global_meter->GetGrossAvailablePower()>expected_consumption); // We are off, turn on only if we have enough power to offload
}

bool DeviceCMV::state_on_condition() const
{
	double max_moisture = 0;
	for(auto ht_device_id : ht_device_ids)
	{
		auto ht = DevicesHT::GetInstance()->GetByID(ht_device_id);
		if(ht->GetHumidity()>max_moisture)
			max_moisture = ht->GetHumidity();
	}

	if(IsForced())
		return (max_moisture>force_max_moisture);

	if(WantOffload())
		return (max_moisture>offload_max_moisture);

	return false;
}

}


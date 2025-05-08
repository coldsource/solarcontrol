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

#include <device/electrical/DeviceTimeRange.hpp>
#include <control/OnOff.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>
#include <set>

using namespace std;
using nlohmann::json;
using datetime::Timestamp;

namespace device {

DeviceTimeRange::DeviceTimeRange(unsigned int id, const string &name, const configuration::Json &config): DeviceOnOff(id, name,config)
{
	for(auto it : config.GetArray("force", json::array()))
		force.push_back(datetime::TimeRange(it));

	for(auto it : config.GetArray("offload", json::array()))
		offload.push_back(datetime::TimeRange(it));

	for(auto it : config.GetArray("remainder", json::array()))
		remainder.push_back(datetime::TimeRange(it));

	min_on_time = config.GetInt("min_on_time", 0);
	min_on_for_last = config.GetInt("min_on_for_last", 0);

	min_on = config.GetInt("min_on", 0);
	max_on = config.GetInt("max_on", 0);
	min_off = config.GetInt("min_off", 0);
}

DeviceTimeRange::~DeviceTimeRange()
{
}

void DeviceTimeRange::CheckConfig(const configuration::Json &conf)
{
	DeviceOnOff::CheckConfig(conf);

	conf.Check("force", "array", false);
	conf.Check("offload", "array", false);

	conf.Check("remainder", "array", false);
	conf.Check("min_on_time", "int", false);
	conf.Check("min_on_for_last", "int", false);

	conf.Check("min_on", "int", false);
	conf.Check("max_on", "int", false);
	conf.Check("min_off", "int", false);
}

bool DeviceTimeRange::IsForced(configuration::Json *data_ptr) const
{
	return force.IsActive(data_ptr);
}

bool DeviceTimeRange::WantOffload(configuration::Json *data_ptr) const
{
	return offload.IsActive(data_ptr);
}

bool DeviceTimeRange::WantRemainder(configuration::Json *data_ptr) const
{
	return remainder.IsActive(data_ptr) && on_history.GetTotalForLast(min_on_for_last)<min_on_time;
}

en_wanted_state DeviceTimeRange::get_wanted_state(configuration::Json *data_ptr) const
{
	if(manual)
		return UNCHANGED;

	Timestamp now(TS_MONOTONIC);
	if(GetState() && now-last_on<min_on)
		return UNCHANGED; // Stay on at least 'min_on' seconds

	if(GetState() && max_on>0 && now-last_on>max_on)
		return OFF; // Stay on no longer than 'max_on' seconds

	if(!GetState() && now-last_off<min_off)
		return UNCHANGED; // Stay of at least 'min_off' seconds

	if(IsForced(data_ptr) || WantRemainder(data_ptr))
		return ON;

	if(WantOffload(data_ptr))
		return OFFLOAD;

	return OFF;
}

en_wanted_state DeviceTimeRange::GetWantedState() const
{
	return get_wanted_state();
}

}

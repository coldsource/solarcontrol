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
#include <configuration/Json.hpp>

#include <stdexcept>
#include <set>

using namespace std;
using nlohmann::json;
using datetime::Timestamp;

namespace device {

DeviceTimeRange::DeviceTimeRange(int id): DeviceOnOff(id)
{
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

void DeviceTimeRange::reload(const configuration::Json &config)
{
	DeviceOnOff::reload(config);

	for(auto it : config.GetArray("force", json::array()))
		force.push_back(datetime::TimeRange(it));

	for(auto it : config.GetArray("offload", json::array()))
		offload.push_back(datetime::TimeRange(it));

	for(auto it : config.GetArray("remainder", json::array()))
		remainder.push_back(datetime::TimeRange(it));

	min_on_time = config.GetUInt("min_on_time", 0);
	min_on_for_last = config.GetUInt("min_on_for_last", 0);

	min_on = config.GetUInt("min_on", 0);
	max_on = config.GetUInt("max_on", 0);
	min_off = config.GetUInt("min_off", 0);
}

bool DeviceTimeRange::IsForced(configuration::Json *data_ptr) const
{
	unique_lock<recursive_mutex> llock(lock);

	return force.IsActive(data_ptr);
}

bool DeviceTimeRange::WantOffload(configuration::Json *data_ptr) const
{
	unique_lock<recursive_mutex> llock(lock);

	return offload.IsActive(data_ptr);
}

bool DeviceTimeRange::WantRemainder(configuration::Json *data_ptr) const
{
	unique_lock<recursive_mutex> llock(lock);

	return remainder.IsActive(data_ptr) && on_history.GetTotalForLast(min_on_for_last)<min_on_time;
}

en_wanted_state DeviceTimeRange::get_wanted_state(configuration::Json *data_ptr) const
{
	if(manual)
		return UNCHANGED;

	Timestamp now(TS_MONOTONIC);
	if(GetState() && (unsigned long)(now-last_on)<min_on)
		return UNCHANGED; // Stay on at least 'min_on' seconds

	if(GetState() && max_on>0 && (unsigned long)(now-last_on)>max_on)
		return OFF; // Stay on no longer than 'max_on' seconds

	if(!GetState() && (unsigned long)(now-last_off)<min_off)
		return UNCHANGED; // Stay of at least 'min_off' seconds

	if(IsForced(data_ptr) || WantRemainder(data_ptr))
		return ON;

	if(WantOffload(data_ptr))
		return OFFLOAD;

	return OFF;
}

en_wanted_state DeviceTimeRange::GetWantedState() const
{
	unique_lock<recursive_mutex> llock(lock);

	return get_wanted_state();
}

}

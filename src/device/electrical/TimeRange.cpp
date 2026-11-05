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

#include <device/electrical/TimeRange.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>
#include <set>

using namespace std;
using nlohmann::json;
using datetime::Timestamp;

namespace device {

TimeRange::TimeRange(int id): OnOff(id)
{
}

TimeRange::~TimeRange()
{
}

void TimeRange::CheckConfig(const configuration::Json &conf)
{
	OnOff::CheckConfig(conf);

	conf.Check("force", "array", false);
	conf.Check("offload", "array", false);

	conf.Check("remainder", "array", false);
	conf.Check("min_on_time", "uint", false);
	conf.Check("min_on_for_last", "uint", false);

	conf.Check("min_on", "uint", false);
	conf.Check("max_on", "uint", false);
	conf.Check("min_off", "uint", false);
}

shared_ptr<Device::Configuration> TimeRange::build_config(const configuration::Json &j_config) const
{
	return make_shared<TimeRange::Configuration>(j_config);
}

TimeRange::Configuration::Configuration(const configuration::Json &config):OnOff::Configuration(config)
{
	force.clear();
	for(auto it : config.GetArray("force", json::array()))
		force.push_back(datetime::TimeRange(it));

	offload.clear();
	for(auto it : config.GetArray("offload", json::array()))
		offload.push_back(datetime::TimeRange(it));

	remainder.clear();
	for(auto it : config.GetArray("remainder", json::array()))
		remainder.push_back(datetime::TimeRange(it));

	min_on_time = config.GetUInt("min_on_time", 0);
	min_on_for_last = config.GetUInt("min_on_for_last", 0);

	min_on = config.GetUInt("min_on", 0);
	max_on = config.GetUInt("max_on", 0);
	min_off = config.GetUInt("min_off", 0);
}

unsigned long TimeRange::GetMinOn()
{
	return get_config()->min_on;
}

bool TimeRange::IsForced(configuration::Json *data_ptr) const
{
	return get_config()->force.IsActive(data_ptr);
}

bool TimeRange::WantOffload(configuration::Json *data_ptr) const
{
	return get_config()->offload.IsActive(data_ptr);
}

bool TimeRange::WantRemainder(configuration::Json *data_ptr) const
{
	auto lconfig = dynamic_pointer_cast<const TimeRange::Configuration>(config.load());
	return lconfig->remainder.IsActive(data_ptr) && GetOnHistoryTotalForLast(lconfig->min_on_for_last)<lconfig->min_on_time;
}

en_wanted_state TimeRange::get_wanted_state(configuration::Json *data_ptr) const
{
	auto lconfig = get_config();
	auto last_on = GetLastOn();
	auto last_off = GetLastOff();

	if(IsManual())
		return UNCHANGED;

	Timestamp now(TS_MONOTONIC);
	if(GetState() && (unsigned long)(now-last_on)<lconfig->min_on)
		return UNCHANGED; // Stay on at least 'min_on' seconds

	if(GetState() && lconfig->max_on>0 && (unsigned long)(now-last_on)>lconfig->max_on)
		return OFF; // Stay on no longer than 'max_on' seconds

	if(!GetState() && (unsigned long)(now-last_off)<lconfig->min_off)
		return UNCHANGED; // Stay of at least 'min_off' seconds

	if(IsForced(data_ptr) || WantRemainder(data_ptr))
		return ON;

	if(WantOffload(data_ptr))
		return OFFLOAD;

	return OFF;
}

en_wanted_state TimeRange::GetWantedState() const
{
	unique_lock<recursive_mutex> llock(lock);

	return get_wanted_state();
}

configuration::Json TimeRange::GetCurrentTimerangeData() const
{
	configuration::Json data;

	if(IsManual())
		return data; // No timerange active since we are in manual mode

	if(IsForced(&data))
		return data;
	if(WantRemainder(&data))
		return data;
	if(WantOffload(&data))
		return data;
	return data;
}

}

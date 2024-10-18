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

#include <device/DeviceTimeRange.hpp>
#include <control/Plug.hpp>
#include <energy/GlobalMeter.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using datetime::Timestamp;

namespace device {

DeviceTimeRange::DeviceTimeRange(unsigned int id, const string &name, const json &config): DeviceOnOff(id, name)
{
	this->global_meter = energy::GlobalMeter::GetInstance();

	check_config_parameters(config, {"prio", "ip", "force", "offload", "expected_consumption", "remainder", "min_on_time", "min_on_for_last", "min_on", "min_off"});

	ctrl = new control::Plug(config["ip"]);

	prio = config["prio"];

	for(auto it : config["force"])
		force.push_back(datetime::TimeRange(datetime::HourMinuteSecond(it["from"]), datetime::HourMinuteSecond(it["to"])));

	for(auto it : config["offload"])
		offload.push_back(datetime::TimeRange(datetime::HourMinuteSecond(it["from"]), datetime::HourMinuteSecond(it["to"])));

	expected_consumption = config["expected_consumption"];

	for(auto it : config["remainder"])
		remainder.push_back(datetime::TimeRange(datetime::HourMinuteSecond(it["from"]), datetime::HourMinuteSecond(it["to"])));

	min_on_time = config["min_on_time"];
	min_on_for_last = config["min_on_for_last"];

	min_on = config["min_on"];
	min_off = config["min_off"];
}

DeviceTimeRange::~DeviceTimeRange()
{
	delete ctrl;
}

bool DeviceTimeRange::IsForced() const
{
	return force.IsActive();
}

bool DeviceTimeRange::WantOffload() const
{
	return (offload.IsActive() && global_meter->GetNetAvailablePower()>expected_consumption);
}

bool DeviceTimeRange::WantRemainder() const
{
	return remainder.IsActive() && on_history.GetTotalForLast(min_on_for_last)<min_on_time;
}

bool DeviceTimeRange::WantedState() const
{
	if(manual)
		return GetState();

	Timestamp now(TS_MONOTONIC);
	if(GetState() && now-last_on<min_on)
		return true;

	if(!GetState() && now-last_off<min_off)
		return false;

	if(state_on_condition())
		return (IsForced() || WantOffload() || WantRemainder());
	else
		return false;
}

bool DeviceTimeRange::GetState() const
{
	return ctrl->GetState();
}

void DeviceTimeRange::SetState(bool new_state)
{
	ctrl->Switch(new_state);

	if(new_state)
		on_history.ClockIn();
	else
		on_history.ClockOut();

	if(new_state)
		last_on = Timestamp(TS_MONOTONIC);
	else
		last_off = Timestamp(TS_MONOTONIC);
}

void DeviceTimeRange::SetManualState(bool new_state)
{
	manual = true;
	SetState(new_state);
}

void DeviceTimeRange::SetAutoState()
{
	manual = false;
}

void DeviceTimeRange::UpdateState()
{
	bool cur_state = ctrl->GetState();
	ctrl->UpdateState();

	bool new_state = ctrl->GetState();

	if(new_state!=cur_state)
		SetState(new_state);
}

}

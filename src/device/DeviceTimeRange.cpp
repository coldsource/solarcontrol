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
#include <energy/Global.hpp>
#include <datetime/Timestamp.hpp>

using namespace std;
using nlohmann::json;

namespace device {

DeviceTimeRange::DeviceTimeRange(const string &name, int prio, const json &config): Device(name, prio)
{
	this->global_meter = energy::Global::GetInstance();
	ctrl = new control::Plug(config["ip"]);

	for(auto it : config["force"])
		force.push_back(datetime::TimeRange(datetime::HourMinuteSecond(it["from"]), datetime::HourMinuteSecond(it["to"])));

	for(auto it : config["offload"])
		offload.push_back(datetime::TimeRange(datetime::HourMinuteSecond(it["from"]), datetime::HourMinuteSecond(it["to"])));

	expected_consumption = config["expected_consumption"];
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
	return offload.IsActive();
}

bool DeviceTimeRange::WantedState() const
{
	if(IsForced())
		return true;

	if(WantOffload() && global_meter->GetNetAvailablePower()>expected_consumption)
		return true;

	return false;
}

bool DeviceTimeRange::GetState() const
{
	return ctrl->GetState();
}

void DeviceTimeRange::SetState(bool new_state)
{
	ctrl->Switch(new_state);
}

}

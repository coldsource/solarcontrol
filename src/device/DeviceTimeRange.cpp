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
#include <control/OnOff.hpp>
#include <energy/GlobalMeter.hpp>
#include <logs/State.hpp>
#include <configuration/Json.hpp>
#include <configuration/ConfigurationSolarControl.hpp>

#include <stdexcept>
#include <set>

using namespace std;
using nlohmann::json;
using datetime::Timestamp;

namespace device {

DeviceTimeRange::DeviceTimeRange(unsigned int id, const string &name, const configuration::Json &config): DeviceOnOff(id, name,config), on_history(id)
{
	this->global_meter = energy::GlobalMeter::GetInstance();

	ctrl = control::OnOff::GetFromConfig(config.GetObject("control"));

	auto scconfig = configuration::ConfigurationSolarControl::GetInstance();
	hysteresis_export = scconfig->GetInt("control.hysteresis.export");
	hysteresis_import = scconfig->GetInt("control.hysteresis.import");

	prio = config.GetInt("prio");

	for(auto it : config.GetArray("force", json::array()))
		force.push_back(datetime::TimeRange(it));

	for(auto it : config.GetArray("offload", json::array()))
		offload.push_back(datetime::TimeRange(it));

	expected_consumption = config.GetInt("expected_consumption", 0);

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
	delete ctrl;
}

bool DeviceTimeRange::IsForced() const
{
	return force.IsActive();
}

bool DeviceTimeRange::WantOffload() const
{
	if(!offload.IsActive())
		return false;

	if(GetState())
		return (global_meter->GetNetAvailablePower(true) > -hysteresis_import); // We are already on, so stay on as long as we have power to offload

	return ((global_meter->GetNetAvailablePower() - hysteresis_export) > expected_consumption); // We are off, turn on only if we have enough power to offload
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
		return true; // Stay on at least 'min_on' seconds

	if(GetState() && max_on>0 && now-last_on>max_on)
		return false; // Stay on no longer than 'max_on' seconds

	if(!GetState() && now-last_off<min_off)
		return false; // Stay of at least 'min_off' seconds

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

	logs::State::LogStateChange(GetID(), manual?logs::State::en_mode::manual:logs::State::en_mode::automatic, new_state);
}

void DeviceTimeRange::SetManualState(bool new_state)
{
	manual = true;
	SetState(new_state);
}

void DeviceTimeRange::SetAutoState()
{
	manual = false;

	logs::State::LogModeChange(GetID(), logs::State::en_mode::automatic);
}

void DeviceTimeRange::UpdateState()
{
	bool cur_state = ctrl->GetState();
	ctrl->UpdateState();

	bool new_state = ctrl->GetState();

	// If we are reloading we do not need to clock in/out as data has already been reloaded from database
	if(new_state!=cur_state && !need_update)
		SetState(new_state);

	need_update = false;
}

}

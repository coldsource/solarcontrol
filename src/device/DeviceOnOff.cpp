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

#include <device/DeviceOnOff.hpp>
#include <control/OnOff.hpp>
#include <meter/Meter.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <logs/State.hpp>
#include <energy/GlobalMeter.hpp>

using namespace std;
using datetime::Timestamp;
using nlohmann::json;

namespace device {

DeviceOnOff::DeviceOnOff(unsigned int id, const std::string &name, const configuration::Json &config):
Device(id, name, config), consumption(id, "consumption"), offload(id, "offload"), on_history(id)
{
	prio = config.GetInt("prio");
	expected_consumption = config.GetInt("expected_consumption", 0);

	ctrl = control::OnOff::GetFromConfig(config.GetObject("control"));
	meter = meter::Meter::GetFromConfig(config.GetObject("control"));

	auto state = state_restore();
	manual = state.GetBool("manual", false);
}

DeviceOnOff::~DeviceOnOff()
{
	json state;
	state["manual"] = manual;
	state_backup(configuration::Json(state));

	delete ctrl;
	delete meter;
}

bool DeviceOnOff::GetState() const
{
	return ctrl->GetState();
}

void DeviceOnOff::SetState(bool new_state)
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

void DeviceOnOff::SetManualState(bool new_state)
{
	manual = true;
	manual_state_changed = true;
	SetState(new_state);
}

void DeviceOnOff::SetAutoState()
{
	manual = false;

	logs::State::LogModeChange(GetID(), logs::State::en_mode::automatic);
}

void DeviceOnOff::UpdateState()
{
	bool cur_state = ctrl->GetState();
	ctrl->UpdateState();

	bool new_state = ctrl->GetState();

	// If we are reloading we do not need to clock in/out as data has already been reloaded from database
	if(new_state!=cur_state && !need_update)
		SetState(new_state);

	need_update = false;
}

double DeviceOnOff::GetExpectedConsumption() const
{
	if(ctrl->GetState() && meter->GetPower()>=0)
		return meter->GetPower();

	return expected_consumption;
}

double DeviceOnOff::GetPower() const
{
	return meter->GetPower();
}

void DeviceOnOff::LogEnergy()
{
	double device_consumption = meter->GetConsumption();
	double pv_ratio = energy::GlobalMeter::GetInstance()->GetPVPowerRatio();

	consumption.AddEnergy(device_consumption);
	offload.AddEnergy(device_consumption * pv_ratio);
}

}

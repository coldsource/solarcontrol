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
#include <logs/State.hpp>

using namespace std;
using datetime::Timestamp;

namespace device {

DeviceOnOff::DeviceOnOff(unsigned int id, const std::string &name, const configuration::Json &config):
Device(id, name, config), consumption(id, "device"), on_history(id)
{
	prio = config.GetInt("prio");
	expected_consumption = config.GetInt("expected_consumption", 0);

	ctrl = control::OnOff::GetFromConfig(config.GetObject("control"));
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
	if(ctrl->GetState() && ctrl->GetPower()>=0)
		return ctrl->GetPower();

	return expected_consumption;
}

double DeviceOnOff::GetPower() const
{
	return ctrl->GetPower();
}

void DeviceOnOff::LogEnergy()
{
	double power = ctrl->GetPower();
	if(power>=0)
		consumption.SetPower(power);
}

}

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

#include <device/electrical/OnOff.hpp>
#include <control/OnOffFactory.hpp>
#include <control/OnOff.hpp>
#include <sensor/sw/SwitchFactory.hpp>
#include <sensor/sw/Switch.hpp>
#include <sensor/meter/MeterFactory.hpp>
#include <sensor/meter/Meter.hpp>
#include <configuration/Json.hpp>
#include <logs/State.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using datetime::Timestamp;
using sensor::meter::MeterFactory;
using sensor::sw::SwitchFactory;
using control::OnOffFactory;
using nlohmann::json;

namespace device {

OnOff::OnOff(int id):
Electrical(id), on_history(id)
{
}

OnOff::~OnOff()
{
}

void OnOff::CheckConfig(const configuration::Json &conf)
{
	conf.Check("control", "object"); // Control is mandatory for OnOff devices

	Electrical::CheckConfig(conf);

	SwitchFactory::CheckConfig(conf.GetObject("control"));
	OnOffFactory::CheckConfig(conf.GetObject("control"));

	conf.Check("prio", "int"); // Prio is mandatory for all onoff devices
	conf.Check("expected_consumption", "int", false);
}

void OnOff::reload(const configuration::Json &config)
{
	Electrical::reload(config);

	prio = config.GetInt("prio");
	expected_consumption = config.GetInt("expected_consumption", 0);

	ctrl = OnOffFactory::GetFromConfig(config.GetObject("control")); // Init control from config
	add_sensor(SwitchFactory::GetFromConfig(config.GetObject("control")), "switch");

	if(!config.Has("meter")) // Device has no dedicated meter, control will be used
		add_sensor(MeterFactory::GetFromConfig(config.GetObject("control")), "meter"); // Fallback on control for metering also
}

void OnOff::state_restore(const  configuration::Json &last_state)
{
	manual = last_state.GetBool("manual", false);
	state = last_state.GetBool("state", false);

	Electrical::state_restore(last_state);
}

configuration::Json OnOff::state_backup()
{
	auto backup = Electrical::state_backup();

	backup.Set("manual", manual);
	backup.Set("state", state);

	return backup;
}

void OnOff::clock(bool new_state)
{
	try
	{
		// Internal function to handle Clock In/Out and last_on/last_off

		if(new_state==state)
			return; // Nothing to do already in current state

		if(new_state)
			on_history.ClockIn();
		else
			on_history.ClockOut();

		if(new_state)
			last_on = Timestamp(TS_MONOTONIC);
		else
			last_off = Timestamp(TS_MONOTONIC);
	}
	catch(...) {} // Ignore clock in/out exception
}

void OnOff::SetState(bool new_state)
{
	unique_lock<recursive_mutex> llock(lock);

	ctrl->Switch(new_state); // Set controller state first, this might fail and throw exception

	clock(new_state);

	state = new_state;

	logs::State::LogStateChange(GetID(), logs::State::en_mode::automatic, new_state);
}

void OnOff::SetManualState(bool new_state)
{
	unique_lock<recursive_mutex> llock(lock);

	ctrl->Switch(new_state); // Set controller state first, this might fail and throw exception

	clock(new_state);

	manual = true;

	state = new_state;

	logs::State::LogStateChange(GetID(), logs::State::en_mode::manual, new_state);
}

void OnOff::SetAutoState()
{
	unique_lock<recursive_mutex> llock(lock);

	manual = false;

	logs::State::LogModeChange(GetID(), logs::State::en_mode::automatic);
}

double OnOff::GetExpectedConsumption() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(GetState() && IsMetered())
		return power; // If device is on and is metered, we take the real consumption

	return expected_consumption; // Take estimated consumption
}

void OnOff::SensorChanged(const sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	const string name = sensor->GetName();
	if(name=="switch")
	{
		auto sw = ((sensor::sw::Switch *)sensor);
		bool new_state = sw->GetState();

		if(sw->IsManual())
			manual = true;

		state = new_state;
	}
	else
		Electrical::SensorChanged(sensor);
}

nlohmann::json OnOff::ToJson() const
{
	auto j_device = Electrical::ToJson();

	j_device["state"] = GetState();
	j_device["manual"] = IsManual();

	return j_device;
}

}

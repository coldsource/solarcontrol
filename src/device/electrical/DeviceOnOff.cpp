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

#include <device/electrical/DeviceOnOff.hpp>
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

DeviceOnOff::DeviceOnOff(int id):
DeviceElectrical(id), on_history(id)
{
}

DeviceOnOff::~DeviceOnOff()
{
}

void DeviceOnOff::CheckConfig(const configuration::Json &conf)
{
	conf.Check("control", "object"); // Control is mandatory for OnOff devices

	DeviceElectrical::CheckConfig(conf);

	SwitchFactory::CheckConfig(conf.GetObject("control"));
	OnOffFactory::CheckConfig(conf.GetObject("control"));

	conf.Check("prio", "int"); // Prio is mandatory for all onoff devices
	conf.Check("expected_consumption", "int", false);
}

void DeviceOnOff::reload(const configuration::Json &config)
{
	DeviceElectrical::reload(config);

	prio = config.GetInt("prio");
	expected_consumption = config.GetInt("expected_consumption", 0);

	ctrl = OnOffFactory::GetFromConfig(config.GetObject("control")); // Init control from config
	add_sensor(SwitchFactory::GetFromConfig(config.GetObject("control")), "switch");

	if(!config.Has("meter")) // Device has no dedicated meter, control will be used
		add_sensor(MeterFactory::GetFromConfig(config.GetObject("control")), "meter"); // Fallback on control for metering also
}

void DeviceOnOff::state_restore(const  configuration::Json &last_state)
{
	manual = last_state.GetBool("manual", false);
	state = last_state.GetBool("state", false);

	DeviceElectrical::state_restore(last_state);
}

configuration::Json DeviceOnOff::state_backup()
{
	auto backup = DeviceElectrical::state_backup();

	backup.Set("manual", manual);
	backup.Set("state", state);

	return backup;
}

void DeviceOnOff::clock(bool new_state)
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

void DeviceOnOff::SetState(bool new_state)
{
	unique_lock<recursive_mutex> llock(lock);

	clock(new_state);

	state = new_state;
	ctrl->Switch(new_state);

	logs::State::LogStateChange(GetID(), logs::State::en_mode::automatic, new_state);
}

void DeviceOnOff::SetManualState(bool new_state)
{
	unique_lock<recursive_mutex> llock(lock);

	clock(new_state);

	manual = true;

	state = new_state;
	ctrl->Switch(new_state);

	logs::State::LogStateChange(GetID(), logs::State::en_mode::manual, new_state);
}

void DeviceOnOff::SetAutoState()
{
	unique_lock<recursive_mutex> llock(lock);

	manual = false;

	logs::State::LogModeChange(GetID(), logs::State::en_mode::automatic);
}

double DeviceOnOff::GetExpectedConsumption() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(GetState() && power>=0)
		return power; // If device is on and is metered, we take the real consumption

	return expected_consumption; // Take estimated consumption
}

void DeviceOnOff::SensorChanged(const sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	const string name = sensor->GetName();
	if(name=="switch")
	{
		bool new_state = ((sensor::sw::Switch *)sensor)->GetState();

		state = new_state;
	}
	else
		DeviceElectrical::SensorChanged(sensor);
}

nlohmann::json DeviceOnOff::ToJson() const
{
	auto j_device = DeviceElectrical::ToJson();

	j_device["state"] = GetState();
	j_device["manual"] = IsManual();

	return j_device;
}

}

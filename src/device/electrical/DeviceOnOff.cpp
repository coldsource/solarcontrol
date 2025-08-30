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
#include <sensor/sw/Switch.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using datetime::Timestamp;
using nlohmann::json;

namespace device {

DeviceOnOff::DeviceOnOff(int id):
DeviceElectrical(id), on_history(id)
{
	auto state_backup = state_restore();
	manual = state_backup.GetBool("manual", false);
	state = state_backup.GetBool("state", false);
}

DeviceOnOff::~DeviceOnOff()
{
	json backup_state;
	backup_state["manual"] = (bool)manual;
	backup_state["state"] = (bool)state;
	state_backup(configuration::Json(backup_state));
}

void DeviceOnOff::CheckConfig(const configuration::Json &conf)
{
	conf.Check("control", "object"); // Control is mandatory for OnOff devices

	DeviceElectrical::CheckConfig(conf);

	conf.Check("prio", "int"); // Prio is mandatory for all onoff devices
	conf.Check("expected_consumption", "int", false);
}

void DeviceOnOff::reload(const configuration::Json &config)
{
	DeviceElectrical::reload(config);

	prio = config.GetInt("prio");
	expected_consumption = config.GetInt("expected_consumption", 0);
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

	DeviceElectrical::SetState(new_state);
}

void DeviceOnOff::SetManualState(bool new_state)
{
	unique_lock<recursive_mutex> llock(lock);

	clock(new_state);

	DeviceElectrical::SetManualState(new_state);
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

		if(new_state==state)
			return; // Already in the good state, state change has been made through SetState() or SetManualState() and we are just getting notification of it

		SetManualState(new_state);
	}
	else
		DeviceElectrical::SensorChanged(sensor);
}

}

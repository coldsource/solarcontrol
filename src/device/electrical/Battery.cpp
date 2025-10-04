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

#include <device/electrical/Battery.hpp>
#include <device/Devices.hpp>
#include <sensor/meter/Voltmeter.hpp>
#include <sensor/sw/Switch.hpp>
#include <control/OnOff.hpp>
#include <configuration/Json.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <database/DB.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;
using sensor::meter::Voltmeter;
using datetime::Timestamp;

namespace device
{

Battery::Battery(int id):OnOff(id, "production", "") // Override default counter for storing production
{
	last_grid_switch = Timestamp(TS_MONOTONIC);

	ObserveConfiguration("control");
}

Battery::~Battery()
{
}

void Battery::ConfigurationChanged(const configuration::ConfigurationPart * config)
{
	unique_lock<recursive_mutex> llock(lock);

	if(config->GetType()=="control")
		battery_cooldown = config->GetTime("control.battery.cooldown");

	OnOff::ConfigurationChanged(config);
}

void Battery::CheckConfig(const configuration::Json &conf)
{
	OnOff::CheckConfig(conf);

	if(conf.Has("voltmeter"))
	{
		conf.Check("voltmeter", "object");
		Voltmeter::CheckConfig(conf.GetObject("voltmeter"));
	}

	conf.Check("policy", "string");
	string_to_policy(conf.GetString("policy")); // Check policy is valid

	// Check battery backup config
	conf.Check("backup", "object");
	auto backup = conf.GetObject("backup");

	backup.Check("battery_low", "uint");
	backup.Check("battery_high", "uint");
	backup.Check("min_grid_time", "uint");
}

Battery::en_battery_policy Battery::string_to_policy(const string &str)
{
	if(str=="grid")
		return GRID;
	else if(str=="battery")
		return BATTERY;
	else if(str=="offload")
		return OFFLOAD;

	throw excpt::Config("Invalid battery policy « " + str + " »", "policy");
}

std::string Battery::policy_to_string(en_battery_policy policy)
{
	if(policy==GRID)
		return "grid";
	else if(policy==BATTERY)
		return "battery";
	return "offload";
}

Battery::en_battery_state Battery::string_to_state(const string &str)
{
	if(str=="discharging")
		return DISCHARGING;
	else if(str=="charging")
		return CHARGING;
	else if(str=="float")
		return FLOAT;
	else if(str=="backup")
		return BACKUP;

	return FLOAT; // Default value, no exception here as it's internal state
}

std::string Battery::state_to_string(en_battery_state state)
{
	if(state==DISCHARGING)
		return "discharging";
	else if(state==CHARGING)
		return "charging";
	else if(state==FLOAT)
		return "float";
	return "backup";
}

void Battery::reload(const configuration::Json &config)
{
	OnOff::reload(config);

	policy = string_to_policy(config.GetString("policy"));

	if(!config.Has("voltmeter"))
	{
		enabled = false;
		return; // Battery is disabled
	}

	enabled = true;

	add_sensor(make_unique<Voltmeter>(config.GetObject("voltmeter")), "voltmeter");

	auto backup = config.GetObject("backup");
	battery_low = backup.GetUInt("battery_low");
	battery_high = backup.GetUInt("battery_high");
	min_grid_time = backup.GetUInt("min_grid_time");

	// Control « reverted » may have changed, force state update
	if(state_restored && ctrl!=nullptr)
		ctrl->Switch(state);
}

void Battery::state_restore(const  configuration::Json &last_state)
{
	voltage = last_state.GetFloat("voltage", 0);
	soc = last_state.GetFloat("soc", 0);
	soc_state = string_to_state(last_state.GetString("soc_state", "float"));

	OnOff::state_restore(last_state);
}

configuration::Json Battery::state_backup()
{
	auto backup = OnOff::state_backup();

	backup.Set("voltage", voltage);
	backup.Set("soc", soc);
	backup.Set("soc_state", state_to_string(soc_state));

	return backup;
}

json Battery::ToJson() const
{
	unique_lock<recursive_mutex> llock(lock);

	json j_device = OnOff::ToJson();

	j_device["voltage"] = voltage;
	j_device["soc"] = soc;
	j_device["soc_state"] = state_to_string(soc_state);
	j_device["state"] = state?"grid":"battery";

	return j_device;
}

void Battery::SensorChanged(const sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	if(sensor->GetName()=="voltmeter")
	{
		Voltmeter *voltmeter = (Voltmeter *)sensor;
		voltage = voltmeter->GetVoltage();
		double old_soc = soc;
		soc = voltmeter->GetSOC();

		// Update soc_state
		if(soc<battery_low)
			soc_state = BACKUP; // Always switch to backup mode if battery is too low

		Timestamp now(TS_MONOTONIC);
		if(soc_state==BACKUP && soc>battery_high && (unsigned long)(now - last_grid_switch) > min_grid_time)
		{
			soc_state = FLOAT; // Switch out of backup state once we have enough charge and last switch was not too recent
			last_grid_switch = now;
		}

		if(soc_state!=BACKUP)
		{
			if(soc_state!=CHARGING && voltmeter->IsCharging())
				soc_state = CHARGING; // Charge start
			else if(soc_state==CHARGING && !voltmeter->IsCharging())
				soc_state = FLOAT; // End of battery charge
			else if(soc<old_soc)
				soc_state = DISCHARGING;
		}
	}
	else
		OnOff::SensorChanged(sensor); // Forward other messages
}

en_wanted_state Battery::GetWantedState() const
{
	// State ON is GRID mode
	// State OFF is BATTERY mode

	unique_lock<recursive_mutex> llock(lock);

	if(ctrl==nullptr)
		return UNCHANGED; // Passive battery

	if(soc==-1)
		return UNCHANGED; // SOC Not yet updated

	if(policy==GRID)
		return ON; // Forced grid mode

	// Battery or Offload policy, see whether we want to use battery

	if(soc_state==BACKUP)
		return ON; // Backup mode is on (not enough power), request forced grid power

	if(policy==BATTERY)
		return OFF; // Battery policy, always used battery (as this stage we know we have enough power)

	Timestamp now(TS_MONOTONIC);
	if(policy==OFFLOAD && (unsigned long)(now-last_on)>battery_cooldown)
		return en_wanted_state::OFFLOAD; // Request offload whenever it is possible

	return UNCHANGED;
}

double Battery::GetExpectedConsumption() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(!GetState() && power>=0)
		return power; // Battery has metered consumption only when on battery

	return expected_consumption; // Take estimated consumption
}

void Battery::CreateInDB()
{
	database::DB db;
	auto res = db.Query("SELECT device_id FROM t_device WHERE device_id=%i"_sql<<DEVICE_ID_BATTERY);
	if(res.FetchRow())
		return; // Already in database

	json config;
	config["prio"] = 1000; // Very low offload priority
	config["policy"] = "battery";

	json backup;
	backup["battery_low"] = 30;
	backup["battery_high"] = 50;
	backup["min_grid_time"] = 7200;
	config["backup"] = backup;

	db.Query("INSERT INTO t_device(device_id, device_type, device_name, device_config) VALUES(%i, 'battery', 'battery', %s)"_sql<<DEVICE_ID_BATTERY<<config.dump());
}

}



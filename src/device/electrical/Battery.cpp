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
#include <device/electrical/Grid.hpp>
#include <device/Devices.hpp>
#include <sensor/voltmeter/Factory.hpp>
#include <sensor/voltmeter/Voltmeter.hpp>
#include <sensor/sw/Switch.hpp>
#include <control/OnOff.hpp>
#include <configuration/Json.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <database/DB.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;
using sensor::voltmeter::Voltmeter;
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
		sensor::voltmeter::Factory::CheckConfig(conf.GetObject("voltmeter"));
	}

	conf.Check("policy", "string");
	string_to_policy(conf.GetString("policy")); // Check policy is valid

	// Check battery backup config
	conf.Check("backup", "object");
	auto backup = conf.GetObject("backup");

	backup.Check("battery_low", "uint");
	backup.Check("battery_high", "uint");
	backup.Check("min_grid_time", "uint");

	unsigned int battery_low = backup.GetUInt("battery_low");
	unsigned int battery_high = backup.GetUInt("battery_high");

	if(battery_low > 100)
		throw excpt::Config("Backup low threshold must be between 0 and 100", "battery_low");

	if(battery_high > 100)
		throw excpt::Config("Backup high threshold must be between 0 and 100", "battery_high");

	if(battery_low >= battery_high)
		throw excpt::Config("Backup low threshold be less than high threshold", "battery_low");

	if(conf.Has("offload"))
	{
		auto offload = conf.GetObject("offload");
		int offload_max = offload.GetPower("max");
		unsigned int offload_soc_low = offload.GetUInt("soc_low");
		unsigned int offload_soc_high = offload.GetUInt("soc_high");

		if(offload_max < 0)
			throw excpt::Config("Offload max must be greater than 0", "offload_max");

		if(offload_soc_low > 100)
			throw excpt::Config("Offload low threshold must be between 0 and 100", "offload_soc_low");

		if(offload_soc_high > 100)
			throw excpt::Config("Offload high threshold must be between 0 and 100", "offload_soc_high");

		if(offload_soc_low >= offload_soc_high)
			throw excpt::Config("Offload low threshold must be less than high threshold", "offload_soc_low");
	}
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

string Battery::policy_to_string(en_battery_policy policy)
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

string Battery::state_to_string(en_battery_state state)
{
	if(state==DISCHARGING)
		return "discharging";
	else if(state==CHARGING)
		return "charging";
	else if(state==FLOAT)
		return "float";
	return "backup";
}

Battery::en_offload_state Battery::string_to_offload_state(const string &str)
{
	if(str=="allowed")
		return ALLOWED;
	return FORBIDDEN;
}

string Battery::offload_state_to_string(en_offload_state state)
{
	if(state==ALLOWED)
		return "allowed";
	return "forbidden";
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

	add_sensor(sensor::voltmeter::Factory::GetFromConfig(config.GetObject("voltmeter")), "voltmeter");

	auto backup = config.GetObject("backup");
	battery_low = backup.GetUInt("battery_low");
	battery_high = backup.GetUInt("battery_high");
	min_grid_time = backup.GetUInt("min_grid_time");

	if(config.Has("offload"))
	{
		auto offload = config.GetObject("offload");
		offload_max = offload.GetPower("max");
		offload_soc_low = offload.GetUInt("soc_low");
		offload_soc_high = offload.GetUInt("soc_high");
	}

	// Control « reverted » may have changed, force state update
	if(state_restored && ctrl!=nullptr)
		ctrl->Switch(state);
}

void Battery::state_restore(const  configuration::Json &last_state)
{
	voltage = last_state.GetFloat("voltage", 0);
	soc = last_state.GetFloat("soc", 0);
	soc_state = string_to_state(last_state.GetString("soc_state", "float"));
	offload_state = string_to_offload_state(last_state.GetString("offload_state", "forbidden"));

	OnOff::state_restore(last_state);
}

configuration::Json Battery::state_backup()
{
	auto backup = OnOff::state_backup();

	backup.Set("voltage", voltage);
	backup.Set("soc", soc);
	backup.Set("soc_state", state_to_string(soc_state));
	backup.Set("offload_state", offload_state_to_string(offload_state));

	return backup;
}

json Battery::ToJson() const
{
	json j_device = OnOff::ToJson();

	j_device["voltage"] = (double)voltage;
	j_device["current"] = (double)current;
	j_device["soc"] = (double)soc;
	j_device["soc_state"] = state_to_string(soc_state);
	j_device["state"] = state?"grid":"battery";
	j_device["offload_state"] = offload_state_to_string(offload_state);
	j_device["output_voltage"] = GetVoltage();
	j_device["output_frequency"] = GetFrequency();

	return j_device;
}

void Battery::SensorChanged(const sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	if(sensor->GetName()=="voltmeter")
	{
		Voltmeter *voltmeter = (Voltmeter *)sensor;
		voltage = voltmeter->GetVoltage();
		current = voltmeter->GetCurrent();
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
			// Try to get direct state from voltmeter first (if supported)
			try
			{
				sensor::voltmeter::Voltmeter::charge_state_t vcs = voltmeter->GetState();
				if(vcs==sensor::voltmeter::Voltmeter::charge_state_t::CHARGING)
					soc_state = CHARGING;
				else if(vcs==sensor::voltmeter::Voltmeter::charge_state_t::DISCHARGING)
					soc_state = DISCHARGING;
				else
					soc_state = FLOAT; // Default to float also for UNKNOWN
			}
			catch(...)
			{
				// Voltmeter doesn't support GetState(), try « guessing »
				if(soc_state!=CHARGING && voltmeter->IsCharging())
					soc_state = CHARGING; // Charge start
				else if(soc_state==CHARGING && !voltmeter->IsCharging())
					soc_state = FLOAT; // End of battery charge
				else if(soc<old_soc)
					soc_state = DISCHARGING;
			}
		}

		// Handle offload hysteresis
		if(offload_max>0)
		{
			if(soc >= offload_soc_high)
				offload_state = ALLOWED;

			if(soc < offload_soc_low)
				offload_state = FORBIDDEN;
		}
	}
	else
		OnOff::SensorChanged(sensor); // Forward other messages
}

en_wanted_state Battery::GetWantedState() const
{
	// Read grid state unlocked
	auto grid_state = Devices::GetByID<Grid>(DEVICE_ID_GRID)->GetState();

	// State ON is GRID mode
	// State OFF is BATTERY mode

	unique_lock<recursive_mutex> llock(lock);

	if(ctrl==nullptr)
		return UNCHANGED; // Passive battery

	if(soc==-1)
		return UNCHANGED; // SOC Not yet updated

	if(manual && soc_state!=BACKUP)
		return UNCHANGED;

	if(grid_state==Grid::en_grid_state::OFFLINE && soc_state!=BACKUP)
		return OFF; // Grid backup mode

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

double Battery::GetPower() const
{
	double power = OnOff::GetPower();
	if(power==-1)
		return -1; // We are unmetered

	// We might see power on our metering device coming from the grid when battery is disabled
	// Force power to 0 if we are disabled or on grid to avoid double counting
	if(!IsOn())
		return 0;

	return power;
}

bool Battery::IsOffloadAllowed() const
{
	unique_lock<recursive_mutex> llock(lock);

	return (offload_max>0 && offload_state==ALLOWED);
}
int Battery::GetOffloadMax() const
{
	unique_lock<recursive_mutex> llock(lock);

	return offload_max;
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



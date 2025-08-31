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

#include <device/electrical/DeviceBattery.hpp>
#include <device/Devices.hpp>
#include <sensor/meter/Voltmeter.hpp>
#include <control/OnOff.hpp>
#include <configuration/Json.hpp>
#include <database/DB.hpp>

using namespace std;
using nlohmann::json;
using sensor::meter::Voltmeter;
using datetime::Timestamp;

namespace device
{

DeviceBattery::DeviceBattery(int id):DevicePassive(id)
{
	// Override default counter for storing production
	consumption = energy::Counter(id, "production");

	auto state = state_restore();
	voltage = state.GetFloat("voltage", 0);
	soc = state.GetFloat("soc", 0);

	last_grid_switch = Timestamp(TS_MONOTONIC);
}

DeviceBattery::~DeviceBattery()
{
	json state;
	state["voltage"] = voltage;
	state["soc"] = soc;
	state_backup(configuration::Json(state));
}

void DeviceBattery::CheckConfig(const configuration::Json &conf)
{
	DevicePassive::CheckConfig(conf);

	conf.Check("voltmeter", "object"); // Meter is mandatory for passive devices
	Voltmeter::CheckConfig(conf.GetObject("voltmeter"));

	// Check battery backup config, passive batteries have no backup so this is optional
	if(conf.Has("backup"))
	{
		conf.Check("backup", "object");
		auto backup = conf.GetObject("backup");

		backup.Check("battery_low", "uint");
		backup.Check("battery_high", "uint");
		backup.Check("min_grid_time", "uint");
	}
}

void DeviceBattery::reload(const configuration::Json &config)
{
	DevicePassive::reload(config);

	add_sensor(make_unique<Voltmeter>(config.GetObject("voltmeter")), "voltmeter");

	if(config.Has("backup"))
	{
		has_backup = true;
		auto backup = config.GetObject("backup");
		battery_low = backup.GetUInt("battery_low");
		battery_high = backup.GetUInt("battery_high");
		min_grid_time = backup.GetUInt("min_grid_time");
	}
}

json DeviceBattery::ToJson() const
{
	unique_lock<recursive_mutex> llock(lock);

	json j_device = DevicePassive::ToJson();

	j_device["voltage"] = voltage;
	j_device["soc"] = soc;
	j_device["state"] = state?"battery":"grid";

	return j_device;
}

void DeviceBattery::SensorChanged(const sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	if(sensor->GetName()=="voltmeter")
	{
		Voltmeter *voltmeter = (Voltmeter *)sensor;
		voltage = voltmeter->GetVoltage();
		soc = voltmeter->GetSOC();
	}
	else
		DevicePassive::SensorChanged(sensor); // Forward messages of meter
}

void DeviceBattery::HandleNonStateActions()
{
	unique_lock<recursive_mutex> llock(lock);

	if(!has_backup)
		return; // We are a passive battery, nothing to do

	if(soc==-1)
		return; // SOC Not yet updated

	// If battery is too low, we always switch back to grid to backup power supply
	if(soc<battery_low && state)
	{
		ctrl->Switch(false);
		return;
	}

	if(soc>=battery_high && !state)
	{
		Timestamp now(TS_MONOTONIC);
		if((unsigned long)(now - last_grid_switch) <= min_grid_time)
			return; // Last grid swich is too recent apply cooldown

		ctrl->Switch(true);
		last_grid_switch = now;
	}
}

void DeviceBattery::CreateInDB()
{
	database::DB db;
	auto res = db.Query("SELECT device_id FROM t_device WHERE device_id=%i"_sql<<DEVICE_ID_BATTERY);
	if(res.FetchRow())
		return; // Already in database

	json config;

	json meter;
	meter["type"] = "dummy";
	meter["mqtt_id"] = "";
	meter["phase"] = "a";
	config["meter"] = meter;

	json voltmeter;
	voltmeter["mqtt_id"] = "";
	voltmeter["thresholds"] = json::array();
	config["voltmeter"] = voltmeter;

	json control;
	control["type"] = "uni";
	control["ip"] = "";
	control["outlet"] = 0;
	config["control"] = control;

	json backup;
	control["battery_low"] = 30;
	control["battery_high"] = 50;
	control["min_grid_time"] = 7200;
	config["backup"] = control;

	db.Query("INSERT INTO t_device(device_id, device_type, device_name, device_config) VALUES(%i, 'battery', 'battery', %s)"_sql<<DEVICE_ID_BATTERY<<config.dump());
}

}



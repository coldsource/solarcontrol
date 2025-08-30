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
#include <configuration/Json.hpp>
#include <database/DB.hpp>

using namespace std;
using nlohmann::json;
using sensor::meter::Voltmeter;

namespace device
{

DeviceBattery::DeviceBattery(int id):DevicePassive(id)
{
	// Override default counter for storing production
	consumption = energy::Counter(id, "production");

	auto state = state_restore();
	voltage = state.GetFloat("voltage", 0);
	soc = state.GetFloat("soc", 0);
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
}

void DeviceBattery::reload(const configuration::Json &config)
{
	DevicePassive::reload(config);

	add_sensor(make_unique<Voltmeter>(config.GetObject("voltmeter")), "voltmeter");
}

json DeviceBattery::ToJson() const
{
	json j_device = DevicePassive::ToJson();

	j_device["voltage"] = (double)voltage;
	j_device["soc"] = (double)soc;

	return j_device;
}

void DeviceBattery::SensorChanged(const sensor::Sensor *sensor)
{
	if(sensor->GetName()=="voltmeter")
	{
		Voltmeter *voltmeter = (Voltmeter *)sensor;
		voltage = voltmeter->GetVoltage();
		soc = voltmeter->GetSOC();
	}
	else
		DevicePassive::SensorChanged(sensor); // Forward messages of meter
}

void DeviceBattery::CreateInDB()
{
	database::DB db;
	auto res = db.Query("SELECT device_id FROM t_device WHERE device_type='battery'"_sql);
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

	db.Query("INSERT INTO t_device(device_id, device_type, device_name, device_config) VALUES(%i, 'battery', 'battery', %s)"_sql<<DEVICE_ID_BATTERY<<config.dump());
}

}



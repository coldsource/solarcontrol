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

#include <device/electrical/BatteryPassive.hpp>
#include <sensor/meter/Voltmeter.hpp>
#include <configuration/Json.hpp>

using namespace std;
using nlohmann::json;
using sensor::meter::Voltmeter;
using datetime::Timestamp;

namespace device
{

BatteryPassive::BatteryPassive(int id):Passive(id)
{
	// Override default counter for storing production
	consumption = energy::Counter(id, "production");
}

BatteryPassive::~BatteryPassive()
{
}

void BatteryPassive::CheckConfig(const configuration::Json &conf)
{
	Passive::CheckConfig(conf);

	conf.Check("voltmeter", "object"); // Voltmeter is mandatory for battery
	Voltmeter::CheckConfig(conf.GetObject("voltmeter"));
}

void BatteryPassive::reload(const configuration::Json &config)
{
	Passive::reload(config);

	add_sensor(make_unique<Voltmeter>(config.GetObject("voltmeter")), "voltmeter");
}

void BatteryPassive::state_restore(const  configuration::Json &last_state)
{
	voltage = last_state.GetFloat("voltage", 0);
	soc = last_state.GetFloat("soc", 0);

	Passive::state_restore(last_state);
}

configuration::Json BatteryPassive::state_backup()
{
	auto backup = Passive::state_backup();

	backup.Set("voltage", voltage);
	backup.Set("soc", soc);

	return backup;
}

json BatteryPassive::ToJson() const
{
	unique_lock<recursive_mutex> llock(lock);

	json j_device = Passive::ToJson();

	j_device["voltage"] = voltage;
	j_device["soc"] = soc;

	return j_device;
}

void BatteryPassive::SensorChanged(const sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	if(sensor->GetName()=="voltmeter")
	{
		Voltmeter *voltmeter = (Voltmeter *)sensor;
		voltage = voltmeter->GetVoltage();
		soc = voltmeter->GetSOC();
	}
	else
		Passive::SensorChanged(sensor); // Forward other messages
}

}




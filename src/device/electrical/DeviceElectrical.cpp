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

#include <device/electrical/DeviceElectrical.hpp>
#include <configuration/Json.hpp>
#include <control/OnOffFactory.hpp>
#include <control/OnOff.hpp>
#include <sensor/sw/SwitchFactory.hpp>
#include <sensor/sw/Switch.hpp>
#include <sensor/meter/MeterFactory.hpp>
#include <sensor/meter/Meter.hpp>
#include <energy/GlobalMeter.hpp>
#include <logs/State.hpp>

using namespace std;
using sensor::sw::SwitchFactory;
using sensor::meter::MeterFactory;
using control::OnOffFactory;
using datetime::Timestamp;
using nlohmann::json;

namespace device {

DeviceElectrical::DeviceElectrical(int id):
Device(id), consumption(id, "consumption"), offload(id, "offload")
{
}

DeviceElectrical::~DeviceElectrical()
{
}

void DeviceElectrical::CheckConfig(const configuration::Json &conf)
{
	Device::CheckConfig(conf);

	if(conf.Has("control"))
		SwitchFactory::CheckConfig(conf.GetObject("control"));

	if(conf.Has("meter"))
		MeterFactory::CheckConfig(conf.GetObject("meter"));
}

void DeviceElectrical::reload(const configuration::Json &config)
{
	Device::reload(config);

	if(config.Has("control"))
		ctrl = OnOffFactory::GetFromConfig(config.GetObject("control")); // Init control from config
	else
		ctrl = OnOffFactory::GetFromConfig(); // Passive devices have no control, get a dummy controller

	if(config.Has("control"))
		add_sensor(SwitchFactory::GetFromConfig(config.GetObject("control")), "switch");

	if(config.Has("meter"))
		add_sensor(MeterFactory::GetFromConfig(config.GetObject("meter")), "meter"); // Device has a dedicated metering configuration
	else
		add_sensor(MeterFactory::GetFromConfig(config.GetObject("control")), "meter"); // Fallback on control for metering also
}

json DeviceElectrical::ToJson() const
{
	unique_lock<recursive_mutex> llock(lock);

	json j_device;

	j_device["device_id"] = GetID();
	j_device["device_type"] = GetType();
	j_device["device_category"] = GetCategory();
	j_device["device_name"] = GetName();
	j_device["device_config"] = (json)GetConfig();
	j_device["state"] = GetState();
	j_device["manual"] = IsManual();
	j_device["power"] = GetPower();

	return j_device;
}

void DeviceElectrical::SetState(bool new_state)
{
	unique_lock<recursive_mutex> llock(lock);

	state = new_state;
	ctrl->Switch(new_state);

	logs::State::LogStateChange(GetID(), logs::State::en_mode::automatic, new_state);
}

void DeviceElectrical::SetManualState(bool new_state)
{
	unique_lock<recursive_mutex> llock(lock);

	manual = true;

	state = new_state;
	ctrl->Switch(new_state);

	logs::State::LogStateChange(GetID(), logs::State::en_mode::manual, new_state);
}

void DeviceElectrical::SetAutoState()
{
	unique_lock<recursive_mutex> llock(lock);

	manual = false;

	logs::State::LogModeChange(GetID(), logs::State::en_mode::automatic);
}

void DeviceElectrical::SensorChanged(const sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	const string name = sensor->GetName();
	if(name=="switch")
		state = ((sensor::sw::Switch *)sensor)->GetState();
	else if(name=="meter")
	{
		sensor::meter::Meter *meter = (sensor::meter::Meter *)sensor;

		power = meter->GetPower();

		// Log energy
		double device_consumption = meter->GetConsumption();
		double device_excess = meter->GetExcess();
		double pv_ratio = energy::GlobalMeter::GetInstance()->GetPVPowerRatio();

		consumption.AddEnergy(device_consumption, device_excess);
		offload.AddEnergy(device_consumption * pv_ratio);
	}
}

}



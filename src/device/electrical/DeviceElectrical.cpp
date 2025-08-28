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
#include <control/Dummy.hpp>
#include <meter/MeterFactory.hpp>
#include <meter/Meter.hpp>
#include <energy/GlobalMeter.hpp>
#include <logs/State.hpp>

using namespace std;
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

void DeviceElectrical::Reload(const string &name, const configuration::Json &config)
{
	unique_lock<recursive_mutex> llock(mutex);

	Device::Reload(name, config);

	if(config.Has("control"))
		ctrl = control::OnOffFactory::GetFromConfig(config.GetObject("control"));
	else
		ctrl = make_shared<control::Dummy>(); // Passive devices have no control

	if(config.Has("meter"))
		meter = meter::MeterFactory::GetFromConfig(config.GetObject("meter")); // Device has a dedicated metering configuration
	else
		meter = meter::MeterFactory::GetFromConfig(config.GetObject("control")); // Fallback on control for metering also
}

double DeviceElectrical::GetPower() const
{
	return meter->GetPower();
}

json DeviceElectrical::ToJson() const
{
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

bool DeviceElectrical::GetState() const
{
	return ctrl->GetState();
}

void DeviceElectrical::SetState(bool new_state)
{
	ctrl->Switch(new_state);

	logs::State::LogStateChange(GetID(), manual?logs::State::en_mode::manual:logs::State::en_mode::automatic, new_state);
}

void DeviceElectrical::SetManualState(bool new_state)
{
	manual = true;
	SetState(new_state);
}

void DeviceElectrical::SetAutoState()
{
	manual = false;

	logs::State::LogModeChange(GetID(), logs::State::en_mode::automatic);
}

void DeviceElectrical::LogEnergy()
{
	double device_consumption = meter->GetConsumption();
	double device_excess = meter->GetExcess();
	double pv_ratio = energy::GlobalMeter::GetInstance()->GetPVPowerRatio();

	consumption.AddEnergy(device_consumption, device_excess);
	offload.AddEnergy(device_consumption * pv_ratio);
}

}



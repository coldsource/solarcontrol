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

#include <device/DeviceElectrical.hpp>
#include <configuration/Json.hpp>
#include <control/OnOff.hpp>
#include <control/Dummy.hpp>
#include <meter/Meter.hpp>
#include <energy/GlobalMeter.hpp>
#include <logs/State.hpp>

using namespace std;
using datetime::Timestamp;
using nlohmann::json;

namespace device {

DeviceElectrical::DeviceElectrical(unsigned int id, const std::string &name, const configuration::Json &config):
Device(id, name, config), consumption(id, "consumption"), offload(id, "offload")
{
	if(config.Has("control"))
		ctrl = control::OnOff::GetFromConfig(config.GetObject("control"));
	else
		ctrl = new control::Dummy(); // Passive devices have no control

	if(config.Has("meter"))
		meter = meter::Meter::GetFromConfig(config.GetObject("meter")); // Device has a dedicated metering configuration
	else
		meter = meter::Meter::GetFromConfig(config.GetObject("control")); // Fallback on control for metering also
}

DeviceElectrical::~DeviceElectrical()
{
	delete ctrl;
	delete meter;
}

double DeviceElectrical::GetPower() const
{
	return meter->GetPower();
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



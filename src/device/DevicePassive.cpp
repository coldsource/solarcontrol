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

#include <device/DevicePassive.hpp>
#include <configuration/Json.hpp>
#include <energy/GlobalMeter.hpp>
#include <meter/Meter.hpp>

using namespace std;
using datetime::Timestamp;
using nlohmann::json;

namespace device {

DevicePassive::DevicePassive(unsigned int id, const std::string &name, const configuration::Json &config):
Device(id, name, config), consumption(id, "consumption"), offload(id, "offload")
{
	meter = meter::Meter::GetFromConfig(config.GetObject("control"));
}

DevicePassive::~DevicePassive()
{
	delete meter;
}

double DevicePassive::GetPower() const
{
	return meter->GetPower();
}

void DevicePassive::LogEnergy()
{
	double device_consumption = meter->GetConsumption();
	double pv_ratio = energy::GlobalMeter::GetInstance()->GetPVPowerRatio();

	consumption.AddEnergy(device_consumption);
	offload.AddEnergy(device_consumption * pv_ratio);
}

}


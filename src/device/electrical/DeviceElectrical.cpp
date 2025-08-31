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
#include <sensor/meter/MeterFactory.hpp>
#include <sensor/meter/Meter.hpp>
#include <energy/GlobalMeter.hpp>

using namespace std;
using sensor::meter::MeterFactory;
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

	if(conf.Has("meter"))
		MeterFactory::CheckConfig(conf.GetObject("meter"));
}

void DeviceElectrical::reload(const configuration::Json &config)
{
	Device::reload(config);

	if(config.Has("meter"))
		add_sensor(MeterFactory::GetFromConfig(config.GetObject("meter")), "meter"); // Device has a dedicated metering configuration
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
	j_device["power"] = GetPower();

	return j_device;
}

void DeviceElectrical::SensorChanged(const sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	const string name = sensor->GetName();
	if(name=="meter")
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



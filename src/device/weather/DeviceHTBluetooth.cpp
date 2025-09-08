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


#include <device/weather/DeviceHTBluetooth.hpp>
#include <sensor/weather/HTBluetooth.hpp>
#include <configuration/Json.hpp>

using namespace std;

namespace device
{

DeviceHTBluetooth::DeviceHTBluetooth(int id):DeviceHT(id)
{
}

DeviceHTBluetooth::~DeviceHTBluetooth()
{
}

void DeviceHTBluetooth::CheckConfig(const configuration::Json &conf)
{
	DeviceHT::CheckConfig(conf);

	sensor::weather::HTBluetooth::CheckConfig(conf);
}

void DeviceHTBluetooth::reload(const configuration::Json &config)
{
	DeviceHT::reload(config);

	ble_addr = config.GetString("ble_addr");
	add_sensor(make_shared<sensor::weather::HTBluetooth>(ble_addr), "ht");
}

void DeviceHTBluetooth::SensorChanged(const sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	sensor::weather::HTBluetooth * htble = (sensor::weather::HTBluetooth *)sensor;
	temperature = htble->GetTemperature();
	humidity = htble->GetHumidity();

	history.Add(weather::MinMaxHT(humidity, temperature));
}

}


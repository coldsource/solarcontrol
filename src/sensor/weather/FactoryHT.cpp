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

#include <sensor/weather/FactoryHT.hpp>
#include <sensor/weather/HTBluetooth.hpp>
#include <sensor/weather/HTWifi.hpp>
#include <configuration/Json.hpp>
#include <excpt/Context.hpp>
#include <excpt/Config.hpp>

using namespace std;

namespace sensor::weather
{

shared_ptr<sensor::Sensor> FactoryHT::get_from_config(const std::string &name, SensorObserver *observer, const configuration::Json &conf)
{
	CheckConfig(conf);

	if(conf.Has("mqtt_id"))
		return make_shared<HTWifi>(name, observer, conf.GetString("mqtt_id"));
	if(conf.Has("ble_addr"))
		return make_shared<HTBluetooth>(name, observer, conf.GetString("ble_addr"));

	return nullptr;
}

void FactoryHT::CheckConfig(const configuration::Json &conf)
{
	excpt::Context ctx("voltmeter", "In voltmeter configuration");

	if(conf.Has("mqtt_id"))
		HTWifi::CheckConfig(conf);
	else if(conf.Has("ble_addr"))
		HTBluetooth::CheckConfig(conf);
	else
		throw excpt::Config("Unknown weather configuration type", "type");
}

}




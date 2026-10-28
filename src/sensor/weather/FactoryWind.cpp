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

#include <sensor/weather/FactoryWind.hpp>
#include <sensor/weather/Wind.hpp>
#include <configuration/Json.hpp>
#include <excpt/Context.hpp>
#include <excpt/Config.hpp>

using namespace std;

namespace sensor::weather
{

shared_ptr<sensor::Sensor> FactoryWind::get_from_config(const std::string &name, SensorObserver *observer, const configuration::Json &conf)
{
	CheckConfig(conf);

	if(conf.Has("mqtt_id"))
		return make_shared<Wind>(name, observer, conf.GetString("mqtt_id"));

	return nullptr;
}

void FactoryWind::CheckConfig(const configuration::Json &conf)
{
	excpt::Context ctx("voltmeter", "In voltmeter configuration");

	if(conf.Has("mqtt_id"))
		Wind::CheckConfig(conf);
	else
		throw excpt::Config("Unknown weather configuration type", "type");
}

}





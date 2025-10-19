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

#include <sensor/meter/Factory.hpp>
#include <sensor/meter/Plug.hpp>
#include <sensor/meter/Pro3EM.hpp>
#include <sensor/meter/ProEM.hpp>
#include <configuration/Json.hpp>
#include <excpt/Context.hpp>
#include <excpt/Config.hpp>

using namespace std;

namespace sensor::meter {

shared_ptr<Meter> Factory::GetFromConfig(const configuration::Json &conf)
{
	CheckConfig(conf);

	string type = conf.GetString("type");

	if(!conf.Has("mqtt_id") || conf.GetString("mqtt_id")=="")
		return nullptr; // No MQTT ID set, metering is disabled

	if(type=="plug")
		return make_shared<Plug>(conf.GetString("mqtt_id"));
	if(type=="pro")
		return nullptr; // Shelly Pro has no energy measurement
	if(type=="uni")
		return nullptr; // Shelly uni has no energy measurement
	if(type=="3em")
		return make_shared<Pro3EM>(conf.GetString("mqtt_id"), conf.GetString("phase"));
	if(type=="em")
		return make_shared<ProEM>(conf.GetString("mqtt_id"), conf.GetString("phase"));
	if(type=="arduino")
		return nullptr; // Arduino has no energy measurement
	if(type=="bsblan")
		return nullptr; // BSBLan has no energy measurement
	if(type=="dummy")
		return nullptr;

	return nullptr;
}

void Factory::CheckConfig(const configuration::Json &conf)
{
	excpt::Context ctx("meter", "In meter configuration");

	string type = conf.GetString("type");

	if(type=="plug")
		return Plug::CheckConfig(conf);
	if(type=="pro")
		return;
	if(type=="uni")
		return;
	if(type=="3em")
		return Pro3EM::CheckConfig(conf);
	if(type=="em")
		return ProEM::CheckConfig(conf);
	if(type=="arduino")
		return;
	if(type=="bsblan")
		return;
	if(type=="dummy")
		return;

	throw excpt::Config("Unknown meter type « " + type + " »", "type");
}

}


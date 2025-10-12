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

#include <sensor/voltmeter/Factory.hpp>
#include <sensor/voltmeter/Uni.hpp>
#include <sensor/voltmeter/Arduino.hpp>
#include <configuration/Json.hpp>
#include <excpt/Context.hpp>
#include <excpt/Config.hpp>

#include <string>

using namespace std;

namespace sensor::voltmeter
{

shared_ptr<Voltmeter> Factory::GetFromConfig(const configuration::Json &conf)
{
	CheckConfig(conf);

	string type = conf.GetString("type");

	if(type=="uni")
		return make_shared<Uni>(conf);
	if(type=="arduino")
		return make_shared<Arduino>(conf);

	return nullptr;
}

void Factory::CheckConfig(const configuration::Json &conf)
{
	excpt::Context ctx("voltmeter", "In voltmeter configuration");

	string type = conf.GetString("type");

	if(type=="uni")
		Uni::CheckConfig(conf);
	else if(type=="arduino")
		Arduino::CheckConfig(conf);
	else
		throw excpt::Config("Unknown voltmeter type « " + type + " »", "type");
}

}



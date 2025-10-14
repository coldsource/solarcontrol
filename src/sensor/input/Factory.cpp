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

#include <sensor/input/Factory.hpp>
#include <sensor/input/Input.hpp>
#include <sensor/input/Plus1PM.hpp>
#include <sensor/input/Pro.hpp>
#include <sensor/input/Grid.hpp>
#include <configuration/Json.hpp>
#include <excpt/Context.hpp>
#include <excpt/Config.hpp>

using namespace std;

namespace sensor::input {

shared_ptr<Input> Factory::GetFromConfig(const configuration::Json &conf)
{
	CheckConfig(conf);

	string type = conf.GetString("type");

	if(type=="dummy")
		return nullptr;
	if(type=="plus1pm")
		return make_shared<Plus1PM>(conf.GetString("mqtt_id", ""), conf.GetString("ip", ""));
	if(type=="pro")
		return make_shared<Pro>(conf.GetString("mqtt_id", ""), conf.GetInt("outlet"), conf.GetString("ip", ""));
	if(type=="grid")
		return make_shared<Grid>(conf.GetString("mqtt_id"));

	return 0;
}

void Factory::CheckConfig(const configuration::Json &conf)
{
	excpt::Context ctx("input", "In input configuration");

	string type = conf.GetString("type");

	if(type=="dummy")
		return;
	if(type=="plus1pm")
		return Plus1PM::CheckConfig(conf);
	else if(type=="pro")
		return Pro::CheckConfig(conf);
	else if(type=="grid")
		return Grid::CheckConfig(conf);

	throw excpt::Config("Unknown input type « " + type + " »", type);
}

}



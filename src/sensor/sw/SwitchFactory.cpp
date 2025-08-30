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

#include <sensor/sw/SwitchFactory.hpp>
#include <sensor/sw/Plug.hpp>
#include <sensor/sw/Pro.hpp>
#include <configuration/Json.hpp>

#include <string>
#include <stdexcept>

using namespace std;

namespace sensor::sw
{

shared_ptr<Switch> SwitchFactory::GetFromConfig(const configuration::Json &conf)
{
	CheckConfig(conf);

	string type = conf.GetString("type");
	if(type=="plug")
		return make_shared<Plug>(conf.GetString("ip"), conf.GetString("mqtt_id", ""));
	if(type=="pro")
		return make_shared<Pro>(conf.GetString("ip"), conf.GetInt("outlet"), conf.GetString("mqtt_id", ""));
	return 0;
}

void SwitchFactory::CheckConfig(const configuration::Json &conf)
{
	string type = conf.GetString("type");

	if(type=="plug")
		Plug::CheckConfig(conf);
	else if(type=="pro")
		Pro::CheckConfig(conf);
	else
		throw invalid_argument("Unknown control type « " + type + " »");
}

}


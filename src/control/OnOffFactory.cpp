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

#include <control/OnOffFactory.hpp>
#include <control/Plug.hpp>
#include <control/Pro.hpp>
#include <configuration/Json.hpp>

#include <string>
#include <stdexcept>

using namespace std;

namespace control
{

OnOff *OnOffFactory::GetFromConfig(const configuration::Json &conf)
{
	CheckConfig(conf);

	string type = conf.GetString("type");
	if(type=="plug")
		return new Plug(conf.GetString("ip"), conf.GetString("mqtt_id", ""));
	if(type=="pro")
		return new Pro(conf.GetString("ip"), conf.GetInt("outlet"), conf.GetString("mqtt_id", ""));
	return 0;
}

void OnOffFactory::CheckConfig(const configuration::Json &conf)
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


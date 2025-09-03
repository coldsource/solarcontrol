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
#include <control/Uni.hpp>
#include <control/Dummy.hpp>
#include <configuration/Json.hpp>

#include <string>
#include <stdexcept>

using namespace std;

namespace control
{

shared_ptr<OnOff> OnOffFactory::GetFromConfig(const configuration::Json &conf)
{
	CheckConfig(conf);

	string type = conf.GetString("type");
	bool reverted = conf.GetBool("reverted", false);

	if(type=="plug")
		return make_shared<Plug>(conf.GetString("ip"), reverted);
	if(type=="pro")
		return make_shared<Pro>(conf.GetString("ip"), conf.GetInt("outlet"), reverted);
	if(type=="uni")
		return make_shared<Uni>(conf.GetString("ip"), conf.GetInt("outlet"), reverted);
	return 0;
}

// Special method to get a dummy controller
shared_ptr<OnOff> OnOffFactory::GetFromConfig()
{
	return make_shared<Dummy>();
}

void OnOffFactory::CheckConfig(const configuration::Json &conf)
{
	string type = conf.GetString("type");

	if(type=="plug")
		Plug::CheckConfig(conf);
	else if(type=="pro")
		Pro::CheckConfig(conf);
	else if(type=="uni")
		Uni::CheckConfig(conf);
	else
		throw invalid_argument("Unknown control type « " + type + " »");
}

}


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

#include <input/InputFactory.hpp>
#include <input/Input.hpp>
#include <input/Dummy.hpp>
#include <input/Plus1PM.hpp>
#include <input/Pro.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;

namespace input {

shared_ptr<Input> InputFactory::GetFromConfig(const configuration::Json &conf)
{
	CheckConfig(conf);

	string type = conf.GetString("type");

	if(type=="dummy")
		return make_shared<Dummy>();
	if(type=="plus1pm")
		return make_shared<Plus1PM>(conf.GetString("mqtt_id", ""), conf.GetString("ip", ""));
	if(type=="pro")
		return make_shared<Pro>(conf.GetString("mqtt_id", ""), conf.GetInt("outlet"), conf.GetString("ip", ""));

	return 0;
}

void InputFactory::CheckConfig(const configuration::Json &conf)
{
	string type = conf.GetString("type");

	if(type=="dummy")
		return Dummy::CheckConfig(conf);
	if(type=="plus1pm")
		return Plus1PM::CheckConfig(conf);
	else if(type=="pro")
		return;

	throw invalid_argument("Unknown input type « " + type + " »");
}

}



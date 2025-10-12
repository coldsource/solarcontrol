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

#include <control/Relay.hpp>
#include <configuration/Json.hpp>
#include <shelly/HTTP.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Config.hpp>
#include <excpt/Context.hpp>

using namespace std;
using nlohmann::json;

namespace control {

void Relay::CheckConfig(const configuration::Json &conf)
{
	OnOff::CheckConfig(conf);

	conf.Check("ip", "string");
	if(conf.GetString("ip")=="")
		throw excpt::Config("Missing IP address", "ip");

	conf.Check("reverted", "bool", false);
}

void Relay::Switch(bool new_state)
{
	excpt::Context ctx("control", "Setting switch state");

	if(reverted)
		new_state = !new_state;

	shelly::HTTP api(ip);

	unique_lock<mutex> llock(lock);

	json j;
	j["id"] = 1;
	j["method"] = "Switch.Set";
	j["params"]["id"] = outlet;
	j["params"]["on"] = new_state;

	api.Post(j);
}

}


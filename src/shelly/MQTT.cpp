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

#include <shelly/MQTT.hpp>
#include <shelly/HTTP.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;

namespace shelly {

json MQTT::Get()
{
	shelly::HTTP http(ip);

	json j;
	j["id"] = 1;
	j["method"] = "MQTT.GetConfig";

	return http.Post(j)["result"];
}

json MQTT::Set(const string &server, const string &topic)
{
	shelly::HTTP http(ip);

	json j;
	j["id"] = 1;
	j["method"] = "MQTT.SetConfig";

	json params;
	params["config"] = json::object();
	params["config"]["enable"] = true;
	params["config"]["server"] = server;
	params["config"]["client_id"] = topic;
	params["config"]["topic_prefix"] = topic;
	j["params"] = params;

	return http.Post(j)["result"];
}

}



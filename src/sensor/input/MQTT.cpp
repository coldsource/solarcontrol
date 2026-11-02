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

#include <sensor/input/MQTT.hpp>
#include <configuration/Json.hpp>
#include <shelly/HTTP.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::input {

MQTT::MQTT(const string &name, const string &mqtt_id, int input, const string &ip):
Input(name),
input(input),
ip(ip)
{
	state = false;

	mqtt_topics = {mqtt_id + "/events/rpc"};
}

MQTT::~MQTT()
{
}

void MQTT::CheckConfig(const configuration::Json &conf)
{
	Input::CheckConfig(conf);

	conf.Check("mqtt_id", "string");
	if(conf.GetString("mqtt_id")=="")
		throw excpt::Config("MQTT ID is required", "mqtt_id");

	conf.Check("ip", "string", false);
}

bool MQTT::get_input() const
{
	if(ip=="")
		return false;

	shelly::HTTP api(ip);

	json j;
	j["id"] = 1;
	j["method"] = "Input.GetStatus";
	j["params"]["id"] = input;

	auto out = api.Post(j);
	return out["result"]["state"];
}

bool MQTT::ForceUpdate()
{
	state = get_input();
	return true;
}

bool MQTT::GetState() const
{
	return state;
}

bool MQTT::HandleMQTTMessage(const string &message, const std::string & /*topic*/)
{
	try
	{
		json j = json::parse(message);
		state = j["params"]["input:" + to_string(input)]["state"];
	}
	catch(json::exception &e)
	{
		return false;
	}

	return true;
}

}



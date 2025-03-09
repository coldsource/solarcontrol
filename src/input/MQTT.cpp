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

#include <input/MQTT.hpp>
#include <logs/Logger.hpp>
#include <mqtt/Client.hpp>
#include <configuration/Json.hpp>
#include <websocket/SolarControl.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace input {

MQTT::MQTT(const string &mqtt_id, int input, const string &ip):
HTTP(ip),
input(input),
topic(mqtt_id + "/events/rpc")
{
	state = false;

	auto mqtt = mqtt::Client::GetInstance();
	mqtt->Subscribe(topic, this);

	UpdateState();
}

MQTT::~MQTT()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic, this);
}

void MQTT::CheckConfig(const configuration::Json &conf)
{
	Input::CheckConfig(conf);

	conf.Check("mqtt_id", "string");
	if(conf.GetString("mqtt_id")=="")
		throw invalid_argument("MQTT ID is required");

	conf.Check("ip", "string", false);
}

bool MQTT::get_input() const
{
	if(ip=="")
		return false;

	json j;
	j["id"] = 1;
	j["method"] = "Input.GetStatus";
	j["params"]["id"] = input;

	try
	{
		auto out = Post(j);
		return out["result"]["state"];
	}
	catch(exception &e)
	{
		logs::Logger::Log(LOG_WARNING, "Unable to get input state : « " + string(e.what()) + " »");
		return false;
	}
}

void MQTT::UpdateState()
{
	unique_lock<mutex> llock(lock);

	state = get_input();
}

bool MQTT::GetState() const
{
	return state;
}

void MQTT::HandleMessage(const string &message)
{
	{
		try
		{
			json j = json::parse(message);
			state = j["params"]["input:" + to_string(input)]["state"];
		}
		catch(json::exception &e)
		{
			return;
		}
	}

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
}

}



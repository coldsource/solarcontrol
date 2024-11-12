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

#include <control/Input.hpp>
#include <logs/Logger.hpp>
#include <mqtt/Client.hpp>
#include <websocket/SolarControl.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace control {

Input::Input(const string &mqtt_id, int input, const string &ip): HTTP(ip), input(input)
{
	state = false;

	auto mqtt = mqtt::Client::GetInstance();
	topic = mqtt_id + "/events/rpc";
	mqtt->Subscribe(topic, this);
}

Input::~Input()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic, this);
}

bool Input::get_input() const
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

void Input::UpdateState()
{
	unique_lock<mutex> llock(lock);

	state = get_input();
}

void Input::HandleMessage(const string &message)
{
	{
		unique_lock<mutex> llock(lock);

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



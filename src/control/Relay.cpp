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
#include <logs/Logger.hpp>
#include <mqtt/Client.hpp>
#include <websocket/SolarControl.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace control {

Relay::Relay(const std::string &ip, int outlet, const string &mqtt_id): HTTP(ip), outlet(outlet)
{
	state = false;

	auto mqtt = mqtt::Client::GetInstance();
	topic = mqtt_id + "/events/rpc";
	mqtt->Subscribe(topic, this);
}

Relay::~Relay()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic, this);
}

void Relay::Switch(bool new_state)
{
	unique_lock<mutex> llock(lock);

	if(ip=="")
		return;

	json j;
	j["id"] = 1;
	j["method"] = "Switch.Set";
	j["params"]["id"] = outlet;
	j["params"]["on"] = new_state;

	try
	{
		Post(j);
	}
	catch(exception &e)
	{
		logs::Logger::Log(LOG_WARNING, "Unable to set plug state : « " + string(e.what()) + " »");
		return;
	}

	state = new_state;
}

bool Relay::get_output() const
{
	if(ip=="")
		return false;

	json j;
	j["id"] = 1;
	j["method"] = "Switch.GetStatus";
	j["params"]["id"] = outlet;

	try
	{
		auto out = Post(j);
		return out["result"]["output"];
	}
	catch(exception &e)
	{
		logs::Logger::Log(LOG_WARNING, "Unable to get plug state : « " + string(e.what()) + " »");
		return false;
	}
}

void Relay::UpdateState()
{
	unique_lock<mutex> llock(lock);

	state = get_output();
}

double Relay::GetPower() const
{
	unique_lock<mutex> llock(lock);

	return power;
}

void Relay::HandleMessage(const string &message)
{
	{
		unique_lock<mutex> llock(lock);

		try
		{
			json j = json::parse(message);
			power = j["params"]["switch:" + to_string(outlet)]["apower"];
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


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

#include <sensor/sw/Relay.hpp>
#include <logs/Logger.hpp>
#include <mqtt/Client.hpp>
#include <configuration/Json.hpp>
#include <shelly/HTTP.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::sw {

Relay::Relay(const std::string &ip, int outlet, const string &mqtt_id):
ip(ip),
outlet(outlet),
topic(mqtt_id!=""?mqtt_id + "/events/rpc":"")
{
	state = false;

	auto mqtt = mqtt::Client::GetInstance();

	if(topic!="")
		mqtt->Subscribe(topic, this);
}

Relay::~Relay()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt && topic!="")
		mqtt->Unsubscribe(topic, this);
}

void Relay::CheckConfig(const configuration::Json &conf)
{
	Switch::CheckConfig(conf);

	conf.Check("ip", "string");
}

bool Relay::get_output() const
{
	if(ip=="")
		return false;

	shelly::HTTP api(ip);

	json j;
	j["id"] = 1;
	j["method"] = "Switch.GetStatus";
	j["params"]["id"] = outlet;

	try
	{
		auto out = api.Post(j);
		return out["result"]["output"];
	}
	catch(exception &e)
	{
		logs::Logger::Log(LOG_WARNING, "Unable to get plug state : « " + string(e.what()) + " »");
		return false;
	}
}

void Relay::ForceUpdate()
{
	{
		unique_lock<mutex> llock(lock);

		state = get_output();
	}

	// Notify observer unlocked
	notify_observer();
}

void Relay::HandleMessage(const string &message, const std::string & /*topic*/)
{
	try
	{

		json j = json::parse(message);
		auto ev = j["params"]["switch:" + to_string(outlet)];
		if(!ev.contains("output"))
			return;

		state = ev["output"];
		notify_observer();
	}
	catch(json::exception &e) {}
}

}


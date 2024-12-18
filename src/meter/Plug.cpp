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

#include <meter/Plug.hpp>
#include <mqtt/Client.hpp>
#include <websocket/SolarControl.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace meter {

Plug::Plug(const string &mqtt_id)
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt_id!="")
		topic = mqtt_id + "/events/rpc";

	if(topic!="")
		mqtt->Subscribe(topic, this);
}

Plug::~Plug()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt && topic!="")
		mqtt->Unsubscribe(topic, this);
}

void Plug::HandleMessage(const string &message)
{
	try
	{
		unique_lock<mutex> llock(lock);

		json j = json::parse(message);

		if(!j.contains("params") || !j["params"].contains("switch:0"))
			return;

		auto ev = j["params"]["switch:0"];

		if(ev.contains("apower"))
			power = ev["apower"];

		if(ev.contains("aenergy"))
			energy_consumption += (double)ev["aenergy"]["by_minute"][0] / 1000;
	}
	catch(json::exception &e)
	{
		return;
	}

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
}

}


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

#include <meter/Pro3EM.hpp>
#include <mqtt/Client.hpp>
#include <websocket/SolarControl.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace meter {

Pro3EM::Pro3EM(const string &mqtt_id, const string &phase): phase(phase)
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt_id!="")
		topic = mqtt_id + "/events/rpc";

	if(topic!="")
		mqtt->Subscribe(topic, this);
}

Pro3EM::~Pro3EM()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt && topic!="")
		mqtt->Unsubscribe(topic, this);
}

void Pro3EM::HandleMessage(const string &message)
{
	try
	{
		unique_lock<mutex> llock(lock);

		json j = json::parse(message);
		power = j["params"]["em:0"][phase + "_act_power"];
	}
	catch(json::exception &e)
	{
		return;
	}

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
}

}



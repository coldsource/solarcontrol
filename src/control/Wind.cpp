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

#include <control/Wind.hpp>
#include <mqtt/Client.hpp>
#include <configuration/Json.hpp>
#include <websocket/SolarControl.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace control
{

Wind::Wind(const string &mqtt_id):topic(mqtt_id + "/events/rpc")
{
	auto mqtt = mqtt::Client::GetInstance();
	mqtt->Subscribe(topic, this);
}

Wind::~Wind()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic, this);
}

void Wind::CheckConfig(const configuration::Json &conf)
{
	conf.Check("mqtt_id", "string");
	if(conf.GetString("mqtt_id")=="")
		throw invalid_argument("MQTT ID is mandatory");
}

double Wind::GetWind() const
{
	return wind;
}

void Wind::SetWind(double w)
{
	unique_lock<mutex> llock(lock);

	wind = w;
}

void Wind::HandleMessage(const string &message, const std::string & /*topic*/)
{
	{
		unique_lock<mutex> llock(lock);

		try
		{
			json j = json::parse(message);
			wind = j["params"]["input:2"]["xfreq"];
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


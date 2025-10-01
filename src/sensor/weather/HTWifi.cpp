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

#include <sensor/weather/HTWifi.hpp>
#include <mqtt/Client.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::weather
{

HTWifi::HTWifi(const string &mqtt_id)
{
	auto mqtt = mqtt::Client::GetInstance();
	topic = mqtt_id + "/events/rpc";
	mqtt->Subscribe(topic, this);
}

HTWifi::~HTWifi()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic, this);
}

void HTWifi::CheckConfig(const configuration::Json &conf)
{
	HT::CheckConfig(conf);

	conf.Check("mqtt_id", "string");
	if(conf.GetString("mqtt_id")=="")
		throw excpt::Config("MQTT ID is mandatory", "mqtt_id");
}

void HTWifi::HandleMessage(const string &message, const std::string & /*topic*/)
{
	{
		try
		{
			json j = json::parse(message);
			humidity = j["params"]["humidity:0"]["rh"];
			temperature = j["params"]["temperature:0"]["tC"];

			notify_observer();
		}
		catch(json::exception &e)
		{
			return;
		}
	}
}

}

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

#include <sensor/voltmeter/Arduino.hpp>
#include <mqtt/Client.hpp>
#include <configuration/Json.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Context.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::voltmeter {

Arduino::Arduino(const configuration::Json &conf):Voltmeter(conf)
{
	CheckConfig(conf);

	string mqtt_id = conf.GetString("mqtt_id");

	auto mqtt = mqtt::Client::GetInstance();
	topic = mqtt_id;
	mqtt->Subscribe(topic, this);
}

Arduino::~Arduino()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic, this);
}

void Arduino::CheckConfig(const configuration::Json &conf)
{
	excpt::Context ctx("meter", "In voltmeter configuration");

	Voltmeter::CheckConfig(conf);

	conf.Check("mqtt_id", "string");

	if(conf.GetString("mqtt_id")=="")
		throw excpt::Config("Missing MQTT ID", "mqtt_id");
}

void Arduino::HandleMessage(const string &message, const std::string & /*topic*/)
{
	try
	{
		unique_lock<mutex> llock(lock);

		json j = json::parse(message);

		if(!j.contains("voltage"))
			return;

		voltage = j["voltage"];
		if(voltage >= max_voltage + charge_delta / 2)
			charging = true;
		else
			charging = false;
	}
	catch(json::exception &e)
	{
		return;
	}

	notify_observer();
}

}

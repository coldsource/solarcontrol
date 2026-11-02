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

#include <sensor/sw/Arduino.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Config.hpp>
#include <excpt/Context.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::sw {

Arduino::Arduino(const std::string &name, const string &mqtt_id, bool reverted):
Switch(name),
reverted(reverted)
{
	state = false;

	mqtt_topics = {mqtt_id};
}

Arduino::~Arduino()
{
}

void Arduino::CheckConfig(const configuration::Json &conf)
{
	Switch::CheckConfig(conf);

	conf.Check("mqtt_id", "string");
	if(conf.GetString("mqtt_id")=="")
		throw excpt::Config("Missing MQTT ID", "mqtt_id");

	conf.Check("reverted", "bool", false);
}

bool Arduino::GetState() const
{
	if(!reverted)
		return state;

	return !state;
}

bool Arduino::HandleMQTTMessage(const string &message, const std::string & /*topic*/)
{
	try
	{
		json j = json::parse(message);
		if(!j.contains("relay"))
			return false;

		state = j["relay"];

		if(j.contains("src") && j["src"]=="btn")
			manual = true;
		else
			manual = false;

		return true;
	}
	catch(json::exception &e)
	{
		return false;
	}
}

}



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

#include <sensor/voltmeter/SC.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Context.hpp>
#include <excpt/Config.hpp>

using namespace std;
using datetime::Timestamp;
using nlohmann::json;

namespace sensor::voltmeter {

SC::SC(const std::string &name, const configuration::Json &conf):Voltmeter(name)
{
	CheckConfig(conf);

	string mqtt_id = conf.GetString("mqtt_id");

	topic_ina = mqtt_id + "/ina236";
	topic_soc = mqtt_id + "/soc";
	topic_temperature = mqtt_id + "/ds18b20";
	mqtt_topics = {topic_ina, topic_soc, topic_temperature};
}

SC::~SC()
{
}

void SC::CheckConfig(const configuration::Json &conf)
{
	excpt::Context ctx("voltmeter", "In voltmeter configuration");

	Voltmeter::CheckConfig(conf);

	conf.Check("mqtt_id", "string");

	if(conf.GetString("mqtt_id")=="")
		throw excpt::Config("Missing MQTT ID", "mqtt_id");
}

bool SC::HandleMQTTMessage(const string &message, const std::string &topic)
{
	Timestamp now(TS_MONOTONIC);

	try
	{
		unique_lock<mutex> llock(lock);

		json j = json::parse(message);

		if(!j.contains("event") || !j.contains("data"))
			return false;

		auto ev = j["data"];

		if(topic==topic_ina)
		{
			v = ev["v"];
			i = ev["i"];
		}
		else if(topic==topic_soc)
		{
			soc = ev["soc"];
			charge_state = ev["state"];
			estimated_time = ev["estimated_time"];
		}
		else if(topic==topic_temperature)
		{
			bool detected = ev["detected"];
			if(!detected)
				temperature = std::numeric_limits<double>::quiet_NaN();
			else
				temperature = ev["temperature"];
		}
	}
	catch(json::exception &e)
	{
		return false;
	}

	// Prevent notify before all states have been read (ina + soc)
	if(soc==-1 || v==-1)
		return false;

	return true;
}

}


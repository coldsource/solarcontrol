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

#include <sensor/voltmeter/Uni.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Context.hpp>
#include <excpt/Config.hpp>

using namespace std;
using datetime::Timestamp;
using nlohmann::json;

namespace sensor::voltmeter {

Uni::Uni(const string &name, const configuration::Json &conf):VoltmeterSimple(name, conf)
{
	CheckConfig(conf);

	string mqtt_id = conf.GetString("mqtt_id");

	mqtt_topics = {mqtt_id + "/events/rpc"};
}

Uni::~Uni()
{
}

void Uni::CheckConfig(const configuration::Json &conf)
{
	excpt::Context ctx("voltmeter", "In voltmeter configuration");

	VoltmeterSimple::CheckConfig(conf);

	conf.Check("mqtt_id", "string");

	if(conf.GetString("mqtt_id")=="")
		throw excpt::Config("Missing MQTT ID", "mqtt_id");
}

bool Uni::HandleMQTTMessage(const string &message, const std::string & /*topic*/)
{
	Timestamp now(TS_MONOTONIC);

	try
	{
		unique_lock<mutex> llock(lock);

		json j = json::parse(message);

		if(!j.contains("params") || !j["params"].contains("voltmeter:100"))
			return false;

		auto ev = j["params"]["voltmeter:100"];

		if(ev.contains("voltage"))
		{
			double voltage = ev["voltage"];
			voltage_avg->Add(voltage, (double)(now - last_voltage_update));
			last_voltage_update = now;

			if(voltage_avg->Get() >= max_voltage + charge_delta / 2)
				charging = true;
			else
				charging = false;
		}
	}
	catch(json::exception &e)
	{
		return false;
	}

	// Notify observer unlocked
	if(!prevent_notify())
		return true;

	return false;
}

}

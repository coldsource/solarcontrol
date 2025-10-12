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
#include <mqtt/Client.hpp>
#include <configuration/Json.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Context.hpp>
#include <excpt/Config.hpp>

using namespace std;
using datetime::Timestamp;
using nlohmann::json;

namespace sensor::voltmeter {

Uni::Uni(const configuration::Json &conf):Voltmeter(conf)
{
	CheckConfig(conf);

	string mqtt_id = conf.GetString("mqtt_id");

	auto mqtt = mqtt::Client::GetInstance();
	topic = mqtt_id + "/events/rpc";
	mqtt->Subscribe(topic, this);

	// Register as configuration observer and trigger ConfigurationChanged() for initial config loading
	ObserveConfiguration("energy");
}

Uni::~Uni()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic, this);
}

void Uni::ConfigurationChanged(const configuration::ConfigurationPart *config)
{
	voltage_avg = make_unique<stat::MovingAverage<double>>(config->GetTime("energy.battery.smoothing"));
	last_voltage_update = Timestamp(TS_MONOTONIC);
}

double Uni::GetVoltage() const
{
	unique_lock<mutex> llock(lock);

	auto avg = voltage_avg.load();

	if(avg->Size()==0)
		return -1; // No measurement yet

	return avg->Get();
}

void Uni::CheckConfig(const configuration::Json &conf)
{
	excpt::Context ctx("voltmeter", "In voltmeter configuration");

	Voltmeter::CheckConfig(conf);

	conf.Check("mqtt_id", "string");

	if(conf.GetString("mqtt_id")=="")
		throw excpt::Config("Missing MQTT ID", "mqtt_id");
}

void Uni::HandleMessage(const string &message, const std::string & /*topic*/)
{
	Timestamp now(TS_MONOTONIC);
	auto avg = voltage_avg.load();

	try
	{
		unique_lock<mutex> llock(lock);

		json j = json::parse(message);

		if(!j.contains("params") || !j["params"].contains("voltmeter:100"))
			return;

		auto ev = j["params"]["voltmeter:100"];

		if(ev.contains("voltage"))
		{
			double voltage = ev["voltage"];
			avg->Add(voltage, (double)(now - last_voltage_update));
			last_voltage_update = now;

			if(avg->Get() >= max_voltage + charge_delta / 2)
				charging = true;
			else
				charging = false;
		}
	}
	catch(json::exception &e)
	{
		return;
	}

	if(avg->Size()>0) // Ensure we have at least one value
	{
		// Notify observer unlocked
		notify_observer();
	}
}

}

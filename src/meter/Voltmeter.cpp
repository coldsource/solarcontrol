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

#include <meter/Voltmeter.hpp>
#include <mqtt/Client.hpp>
#include <websocket/SolarControl.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace meter {

Voltmeter::Voltmeter(const configuration::Json &conf)
{
	string mqtt_id = conf.GetString("mqtt_id");

	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt_id!="")
		topic = mqtt_id + "/events/rpc";

	if(topic!="")
		mqtt->Subscribe(topic, this);

	for(auto it : conf.GetArray("thresholds"))
	{
		int percent = it.GetInt("percent");
		double tvoltage = it.GetFloat("voltage");
		thresholds.insert(pair<int, double>(percent, tvoltage));
	}
}

Voltmeter::~Voltmeter()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt && topic!="")
		mqtt->Unsubscribe(topic, this);
}

void Voltmeter::CheckConfig(const configuration::Json &conf)
{
	conf.Check("mqtt_id", "string", false);
	conf.Check("thresholds", "array");

	int last_percent = -1;
	double last_tvoltage = -1;
	for(auto it : conf.GetArray("thresholds"))
	{
		int percent = it.GetInt("percent");
		double tvoltage = it.GetFloat("voltage");

		if(last_percent==-1 && percent!=0)
			throw invalid_argument("First percent must be 0");

		if(percent<=last_percent)
			throw invalid_argument("Percents must be in increasing order");

		if(tvoltage<=last_tvoltage)
			throw invalid_argument("Voltages must be in increasing order");

		last_percent = percent;
		last_tvoltage = tvoltage;
	}

	if(last_percent!=100)
		throw invalid_argument("Last percent must be 100");
}

double Voltmeter::GetSOC() const
{
	int threshold_percent_start = 0, threshold_percent_end = 0;
	double threshold_voltage_start = 0, threshold_voltage_end = 0;

	for(auto it : thresholds)
	{
		threshold_percent_end = it.first;
		threshold_voltage_end = it.second;

		if(threshold_percent_end==0 && voltage<threshold_voltage_end)
			return 0; // Under minimum voltage

		if(threshold_percent_end==100 && voltage>=threshold_voltage_end)
		{
			if(voltage>=threshold_voltage_end + 0.4)
				return 101; // Charging

			return 100; // Full
		}

		if(threshold_voltage_end>voltage)
			break;

		threshold_percent_start = threshold_percent_end;
		threshold_voltage_start = threshold_voltage_end;
	}

	double threshold_position = (voltage - threshold_voltage_start) / (threshold_voltage_end - threshold_voltage_start);
	return threshold_percent_start + (double)(threshold_percent_end - threshold_percent_start) * threshold_position;
}

void Voltmeter::HandleMessage(const string &message, const std::string & /*topic*/)
{
	try
	{
		unique_lock<mutex> llock(lock);

		json j = json::parse(message);

		if(!j.contains("params") || !j["params"].contains("voltmeter:100"))
			return;

		auto ev = j["params"]["voltmeter:100"];

		if(ev.contains("voltage"))
			voltage = ev["voltage"];
	}
	catch(json::exception &e)
	{
		return;
	}

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
}

}



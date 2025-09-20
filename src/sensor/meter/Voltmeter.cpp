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

#include <sensor/meter/Voltmeter.hpp>
#include <mqtt/Client.hpp>
#include <configuration/Json.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using datetime::Timestamp;
using nlohmann::json;

namespace sensor::meter {

Voltmeter::Voltmeter(const configuration::Json &conf)
{
	string mqtt_id = conf.GetString("mqtt_id");

	auto mqtt = mqtt::Client::GetInstance();
	topic = mqtt_id + "/events/rpc";
	mqtt->Subscribe(topic, this);

	for(auto it : conf.GetArray("thresholds"))
	{
		int percent = it.GetInt("percent");
		double tvoltage = it.GetFloat("voltage");
		thresholds.insert(pair<int, double>(percent, tvoltage));
	}

	// Register as configuration observer and trigger ConfigurationChanged() for initial config loading
	ObserveConfiguration("energy");
}

Voltmeter::~Voltmeter()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic, this);
}

void Voltmeter::ConfigurationChanged(const configuration::ConfigurationPart *config)
{
	voltage_avg = make_unique<stat::MovingAverage<double>>(config->GetTime("energy.battery.smoothing"));
	last_voltage_update = Timestamp(TS_MONOTONIC);
}

double Voltmeter::GetVoltage() const
{
	unique_lock<mutex> llock(lock);

	auto avg = voltage_avg.load();

	if(avg->Size()==0)
		return -1; // No measurement yet

	return avg->Get();
}

void Voltmeter::CheckConfig(const configuration::Json &conf)
{
	conf.Check("mqtt_id", "string");
	conf.Check("thresholds", "array");

	if(conf.GetString("mqtt_id")=="")
		throw invalid_argument("Missing MQTT ID");

	if(conf.GetArray("thresholds").size()==0)
		throw invalid_argument("Battery SOC configuration is empty");

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
	// GetVoltage() will lock, call before our lock
	double voltage = GetVoltage();

	unique_lock<mutex> llock(lock);

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



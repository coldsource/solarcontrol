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

#include <thread/Stats.hpp>
#include <device/Devices.hpp>
#include <device/electrical/DeviceOnOff.hpp>
#include <device/weather/DeviceWeather.hpp>
#include <energy/GlobalMeter.hpp>
#include <websocket/SolarControl.hpp>
#include <logs/Logger.hpp>
#include <configuration/ConfigurationPart.hpp>

#include <stdexcept>

using namespace std;
using namespace device;
using nlohmann::json;

namespace thread {

Stats *Stats::instance = 0;

Stats::Stats()
{
	// Register as configuration observer and trigger ConfigurationChanged() for initial config loading
	ObserveConfiguration("control");

	global_meter = energy::GlobalMeter::GetInstance();

	instance = this;
}

Stats::~Stats()
{
}

void Stats::ConfigurationChanged(const configuration::ConfigurationPart *config)
{
	unique_lock<mutex> llock(lock);

	double offload_max = config->GetPower("control.offload.max");
	slow_time = config->GetTime("control.hysteresis.slow");
	medium_time = config->GetTime("control.hysteresis.medium");
	fast_time = config->GetTime("control.hysteresis.fast");

	hysteresis_export = config->GetPower("control.hysteresis.export");
	controllable_power_avg = make_unique<stat::MovingAverage<double>>(config->GetTime("control.hysteresis.smoothing"));
	controllable_power_frequency_slow = make_unique<stat::CumulativeHigherFrequency<double>>(slow_time, 0, offload_max, 100);
	controllable_power_frequency_medium = make_unique<stat::CumulativeHigherFrequency<double>>(medium_time, 0, offload_max, 100);
	controllable_power_frequency_fast = make_unique<stat::CumulativeHigherFrequency<double>>(fast_time, 0, offload_max, 100);
	production_power_frequency = make_unique<stat::CumulativeHigherFrequency<double>>(slow_time, 0, offload_max, 100);
}

double Stats::get_controlled_power() const
{
	auto onoff = Devices::Get<DeviceOnOff>();

	double controlled_active_power = 0;

	for(auto device : onoff)
	{
		if(device->GetState())
			controlled_active_power += device->GetExpectedConsumption();
	}

	return controlled_active_power;
}

Stats::en_weather_type Stats::get_weather_type() const
{
	int max = 0, total = 0;
	for(auto [range, frequency] : production_power_frequency->GetFrequencies())
	{
		if(frequency==0)
			break;

		if(frequency==1)
			max++;

		total++;
	}

	return (total - max)<=1?CONTANT:VARIABLE;
}

Stats::en_device_speed Stats::get_device_type(const std::shared_ptr<device::DeviceOnOff> &device) const
{
	if(device->GetMinOn()<=fast_time)
		return FAST;

	if(device->GetMinOn()<=medium_time)
		return MEDIUM;

	return SLOW;
}

void Stats::main()
{
	while(true)
	{
		try
		{
			// Lock our config
			unique_lock<mutex> llock(lock);

			controlled_power = get_controlled_power();
			controllable_power = global_meter->GetNetAvailablePower(true) + controlled_power;

			// Compute controllable power history
			controllable_power_frequency_slow->Add(controllable_power);
			controllable_power_frequency_medium->Add(controllable_power);
			controllable_power_frequency_fast->Add(controllable_power);
			production_power_frequency->Add(global_meter->GetPVPower());

			// Compute moving average of available power
			controllable_power_avg->Add(controllable_power);
		}
		catch(exception &e)
		{
			logs::Logger::Log(LOG_ERR, "Statistics exception : « " + string(e.what()) + " »");
		}

		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::STATS);

		if(!wait(1))
			return;
	}
}

nlohmann::json Stats::frequency_to_json(const unique_ptr<stat::CumulativeHigherFrequency<double>> &frequency) const
{
	json j_frequencies = json::array();

	for(auto [range, frequency] : frequency->GetFrequencies())
	{
		json j_frequency = json::object();
		j_frequency["range"] = range;
		j_frequency["frequency"] = frequency;
		j_frequencies.push_back(j_frequency);
	}

	return j_frequencies;
}

json Stats::devices_predictions_to_json() const
{
	auto onoff = Devices::Get<DeviceOnOff>();

	json j_res = json::array();
	for(auto device : onoff)
	{
		if(device->GetID()<0)
			continue; // Skip special devices

		if(device->GetState())
			continue; // Already ON

		json j_device = json::object();

		j_device["device_id"] = device->GetID();
		j_device["device_name"] = device->GetName();

		auto device_type = get_device_type(device);
		if(device_type==SLOW)
			j_device["type"] = "slow";
		else if(device_type==MEDIUM)
			j_device["type"] = "medium";
		else if(device_type==FAST)
			j_device["type"] = "fast";

		j_device["prediction"] = get_device_prediction(device, controlled_power);

		auto wanted_state = device->GetWantedState();
		if(wanted_state==ON)
			j_device["wanted_state"] = "on";
		else if(wanted_state==OFF)
			j_device["wanted_state"] = "off";
		else if(wanted_state==OFFLOAD)
			j_device["wanted_state"] = "offload";
		else if(wanted_state==UNCHANGED)
			j_device["wanted_state"] = "unchanged";

		j_res.push_back(j_device);
	}

	return j_res;
}

double Stats::GetControllablePowerAvg() const
{
	unique_lock<mutex> llock(lock);

	return controllable_power_avg->Get();
}

double Stats::get_device_prediction(const std::shared_ptr<device::DeviceOnOff> &device, double active_power) const
{
	auto device_type = get_device_type(device);
	if(device_type==SLOW)
		return controllable_power_frequency_slow->GetFrequency(device->GetExpectedConsumption() + active_power + hysteresis_export);
	else if(device_type==MEDIUM)
		return controllable_power_frequency_medium->GetFrequency(device->GetExpectedConsumption() + active_power + hysteresis_export);
	else
		return controllable_power_frequency_fast->GetFrequency(device->GetExpectedConsumption() + active_power + hysteresis_export);
}

double Stats::GetDevicePrediction(const std::shared_ptr<device::DeviceOnOff> &device, double active_power) const
{
	unique_lock<mutex> llock(lock);

	return get_device_prediction(device, active_power);
}

json Stats::ToJson() const
{
	unique_lock<mutex> llock(lock);

	json j_state;

	j_state["controllable_power_frequency"]["slow"] = frequency_to_json(controllable_power_frequency_slow);
	j_state["controllable_power_frequency"]["medium"] = frequency_to_json(controllable_power_frequency_medium);
	j_state["controllable_power_frequency"]["fast"] = frequency_to_json(controllable_power_frequency_fast);
	j_state["controllable_power_avg"] = controllable_power_avg->Get();
	j_state["controllable_power"] = controllable_power;
	j_state["controlled_power"] = controlled_power;
	j_state["devices"] = devices_predictions_to_json();
	j_state["weather_type"] = get_weather_type()==VARIABLE?"variable":"constant";

	return j_state;
}

}


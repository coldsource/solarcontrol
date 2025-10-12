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

#include <sensor/voltmeter/Voltmeter.hpp>
#include <configuration/Json.hpp>
#include <excpt/Context.hpp>
#include <excpt/Config.hpp>

using namespace std;

namespace sensor::voltmeter {

Voltmeter::Voltmeter(const configuration::Json &conf)
{
	CheckConfig(conf);

	for(auto it : conf.GetArray("thresholds"))
	{
		int percent = it.GetInt("percent");
		double tvoltage = it.GetFloat("voltage");
		thresholds.insert(pair<int, double>(percent, tvoltage));
	}

	charge_delta = conf.GetFloat("charge_delta");
	max_voltage = thresholds[100];
}

void Voltmeter::CheckConfig(const configuration::Json &conf)
{
	conf.Check("charge_delta", "float");
	conf.Check("thresholds", "array");

	if(conf.GetArray("thresholds").size()==0)
		throw excpt::Config("Battery SOC configuration is empty", "thresholds");

	int last_percent = -1;
	double last_tvoltage = -1;
	for(auto it : conf.GetArray("thresholds"))
	{
		int percent = it.GetInt("percent");
		double tvoltage = it.GetFloat("voltage");

		if(last_percent==-1 && percent!=0)
			throw excpt::Config("First percent must be 0", "thresholds");

		if(percent<=last_percent)
			throw excpt::Config("Percents must be in increasing order", "thresholds");

		if(tvoltage<=last_tvoltage)
			throw excpt::Config("Voltages must be in increasing order", "thresholds");

		last_percent = percent;
		last_tvoltage = tvoltage;
	}

	if(last_percent!=100)
		throw excpt::Config("Last percent must be 100", "thresholds");
}

double Voltmeter::GetSOC() const
{
	// GetVoltage() will lock, call before our lock
	double voltage = GetVoltage();
	if(IsCharging())
		voltage -= charge_delta;

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
			return 100; // Full

		if(threshold_voltage_end>voltage)
			break;

		threshold_percent_start = threshold_percent_end;
		threshold_voltage_start = threshold_voltage_end;
	}

	double threshold_position = (voltage - threshold_voltage_start) / (threshold_voltage_end - threshold_voltage_start);
	return threshold_percent_start + (double)(threshold_percent_end - threshold_percent_start) * threshold_position;
}

}



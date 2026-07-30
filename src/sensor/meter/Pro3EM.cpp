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

#include <sensor/meter/Pro3EM.hpp>
#include <mqtt/Client.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::meter {

Pro3EM::Pro3EM(const string &mqtt_id, const string &phase):phase(phase)
{
	auto mqtt = mqtt::Client::GetInstance();
	topic = mqtt_id + "/events/rpc";
	mqtt->Subscribe(topic, this);
}

Pro3EM::~Pro3EM()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic, this);
}

void Pro3EM::CheckConfig(const configuration::Json &conf)
{
	Meter::CheckConfig(conf);

	string phase = conf.GetString("phase");
	if(phase!="a" && phase!="b" && phase!="c" && phase!="z")
		throw excpt::Config("Phase must be a, b, c or z", "phase");
}

void Pro3EM::HandleMessage(const string &message, const std::string & /*topic*/)
{
	try
	{
		unique_lock<mutex> llock(lock);

		json j = json::parse(message);

		if(j.contains("params") && j["params"].contains("em:0"))
		{
			auto ev = j["params"]["em:0"];

			if(phase=="z")
			{
				power = (double)ev["a_act_power"] + (double)ev["b_act_power"] + (double)ev["c_act_power"];
				voltage = (double)ev["a_voltage"];
				freq = (double)ev["a_freq"];
			}
			else
			{
				power = ev[phase + "_act_power"];
				voltage = ev[phase + "_voltage"];
				freq = ev[phase + "_freq"];
			}
		}

		if(j.contains("params") && j["params"].contains("emdata:0"))
		{
			auto ev = j["params"]["emdata:0"];

			double total_consumption;
			double total_excess;

			if(phase=="z")
			{
				total_consumption = (double)ev["a_total_act_energy"] + (double)ev["b_total_act_energy"] + (double)ev["c_total_act_energy"];
				total_excess = (double)ev["a_total_act_ret_energy"] + (double)ev["b_total_act_ret_energy"] + (double)ev["c_total_act_ret_energy"];
			}
			else
			{
				total_consumption = ev[phase + "_total_act_energy"];
				total_excess = ev[phase + "_total_act_ret_energy"];
			}

			double consumption_delta = last_energy_consumption==0?0:(total_consumption - last_energy_consumption);
			last_energy_consumption = total_consumption;
			if(consumption_delta>0)
				energy_consumption += consumption_delta;

			double excess_delta = last_energy_excess==0?0:(total_excess - last_energy_excess);
			last_energy_excess = total_excess;
			if(excess_delta>0)
				energy_excess += excess_delta;
		}
	}
	catch(json::exception &e)
	{
		return;
	}

	// Call observer unlocked
	notify_observer();
}

}



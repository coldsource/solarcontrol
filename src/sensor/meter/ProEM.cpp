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

#include <sensor/meter/ProEM.hpp>
#include <mqtt/Client.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::meter {

ProEM::ProEM(const string &mqtt_id, const string &phase)
{
	phasei = phase[0] - 'a';

	if(phasei==25) // z is 'All'
		phasei = -1;

	auto mqtt = mqtt::Client::GetInstance();
	topic = mqtt_id + "/events/rpc";
	mqtt->Subscribe(topic, this);
}

ProEM::~ProEM()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic, this);
}

void ProEM::CheckConfig(const configuration::Json &conf)
{
	Meter::CheckConfig(conf);

	string phase = conf.GetString("phase");
	if(phase!="a" && phase!="b" && phase!="z")
		throw excpt::Config("Phase must be a, b or z", "phase");
}

void ProEM::HandleMessage(const string &message, const std::string & /*topic*/)
{
	try
	{
		unique_lock<mutex> llock(lock);

		json j = json::parse(message);

		double total_power = 0;
		for(int i = 0; i < 2; i++)
		{
			// Power
			if((phasei==-1 || i==phasei) && j.contains("params") && j["params"].contains("em1:" + to_string(i)))
			{
				auto ev = j["params"]["em1:" + to_string(i)];

				power_phases[i] = ev["act_power"];
			}

			total_power += power_phases[i];

			// Energy
			if((phasei==-1 || i==phasei) && j.contains("params") && j["params"].contains("em1data:" + to_string(i)))
			{
				auto ev = j["params"]["em1data:" + to_string(i)];

				double total_consumption = ev["total_act_energy"];
				double total_excess = ev["total_act_ret_energy"];

				double consumption_delta = last_energy_consumption_phases[i]==0?0:(total_consumption - last_energy_consumption_phases[i]);
				last_energy_consumption_phases[i] = total_consumption;
				if(consumption_delta>0)
					energy_consumption += consumption_delta;

				double excess_delta = last_energy_excess_phases[i]==0?0:(total_excess - last_energy_excess_phases[i]);
				last_energy_excess_phases[i] = total_excess;
				if(excess_delta>0)
					energy_excess += excess_delta;
			}
		}

		power = total_power;
	}
	catch(json::exception &e)
	{
		return;
	}

	// Call observer unlocked
	notify_observer();
}

}

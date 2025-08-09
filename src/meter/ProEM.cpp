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

#include <meter/ProEM.hpp>
#include <configuration/Json.hpp>
#include <websocket/SolarControl.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace meter {

ProEM::ProEM(const std::string &mqtt_id, const std::string &phase): Pro3EM(mqtt_id, phase)
{
	phasei = phase[0] - 'a';
}

void ProEM::CheckConfig(const configuration::Json &conf)
{
	Meter::CheckConfig(conf);

	string phase = conf.GetString("phase");
	if(phase!="a" && phase!="b")
		throw invalid_argument("Phase must be a or b");
}

void ProEM::HandleMessage(const string &message, const std::string & /*topic*/)
{
	try
	{
		unique_lock<mutex> llock(lock);

		json j = json::parse(message);

		if(j.contains("params") && j["params"].contains("em1:" + to_string(phasei)))
		{
			auto ev = j["params"]["em1:" + to_string(phasei)];

			power = ev["act_power"];
		}

		if(j.contains("params") && j["params"].contains("em1data:" + to_string(phasei)))
		{
			auto ev = j["params"]["em1data:" + to_string(phasei)];

			double total_consumption = ev["total_act_energy"];
			double total_excess = ev["total_act_ret_energy"];

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

	if(websocket::SolarControl::GetInstance())
	{
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::METER);
	}
}

}

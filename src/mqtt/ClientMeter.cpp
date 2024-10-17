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

#include <mqtt/ClientMeter.hpp>
#include <energy/Counter.hpp>
#include <nlohmann/json.hpp>
#include <configuration/ConfigurationSolarControl.hpp>

using namespace std;
using configuration::ConfigurationSolarControl;

using nlohmann::json;

namespace mqtt {

ClientMeter::ClientMeter(): Client(
	ConfigurationSolarControl::GetInstance()->Get("mqtt.host"),
	ConfigurationSolarControl::GetInstance()->GetInt("mqtt.port"),
	ConfigurationSolarControl::GetInstance()->Get("energy.mqtt.id") + "/events/rpc"
)
{

}

void ClientMeter::handle_message(const std::string &message)
{
	double power_grid, power_pv, power_hws;

	try
	{
		json j = json::parse(message);
		power_grid = j["params"]["em:0"]["a_act_power"];
		power_pv = j["params"]["em:0"]["b_act_power"];
		power_hws = j["params"]["em:0"]["c_act_power"];
	}
	catch(json::exception &e)
	{
		return;
	}

	unique_lock<mutex> llock(lock);

	if(grid)
		grid->SetPower(power_grid);

	if(pv)
		pv->SetPower(power_pv);

	if(hws)
		hws->SetPower(power_hws);
}

void ClientMeter::SetCounters(energy::Counter *grid, energy::Counter *pv, energy::Counter *hws)
{
	unique_lock<mutex> llock(lock);

	this->grid = grid;
	this->pv = pv;
	this->hws = hws;
}

}

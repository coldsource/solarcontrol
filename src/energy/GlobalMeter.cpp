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

#include <energy/GlobalMeter.hpp>
#include <energy/Counter.hpp>
#include <nlohmann/json.hpp>
#include <configuration/ConfigurationSolarControl.hpp>
#include <mqtt/Client.hpp>

using namespace std;
using configuration::ConfigurationSolarControl;

using nlohmann::json;

namespace energy {

GlobalMeter * GlobalMeter::instance = 0;

GlobalMeter::GlobalMeter()
:grid("grid", "grid-excess"), pv("pv"), hws("hws")
{
	string topic = ConfigurationSolarControl::GetInstance()->Get("energy.mqtt.id") + "/events/rpc";
	hws_min_energy = configuration::ConfigurationSolarControl::GetInstance()->GetInt("energy.hws.min");

	mqtt::Client::GetInstance()->Subscribe(topic, this);

	instance = this;
}

double GlobalMeter::GetGridPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	return grid.GetPower();
}

double GlobalMeter::GetPVPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	double power = pv.GetPower();
	return power>=0?power:0;
}

double GlobalMeter::GetHWSPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	return hws.GetPower();
}

double GlobalMeter::GetPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	return grid.GetPower() + (pv.GetPower()>0?pv.GetPower():0);
}

double GlobalMeter::GetNetAvailablePower() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(hws.GetEnergyConsumption()>hws_min_energy)
		return GetGrossAvailablePower();
	return GetExcessPower();
}

double GlobalMeter::GetGrossAvailablePower() const
{
	unique_lock<recursive_mutex> llock(lock);

	double available = hws.GetPower()-grid.GetPower();
	return (available<0)?0:available;
}

double GlobalMeter::GetExcessPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	double grid_power = grid.GetPower();
	return (grid_power>0)?0:-grid_power;
}

double GlobalMeter::GetGridEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return grid.GetEnergyConsumption();
}

double GlobalMeter::GetExportedEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return grid.GetEnergyExcess();
}

double GlobalMeter::GetPVEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return pv.GetEnergyConsumption();
}

double GlobalMeter::GetHWSEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return hws.GetEnergyConsumption();
}

void GlobalMeter::SaveHistory()
{
	grid.SaveHistory();
	pv.SaveHistory();
	hws.SaveHistory();
}

void GlobalMeter::HandleMessage(const std::string &message)
{
	unique_lock<recursive_mutex> llock(lock);

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

	grid.SetPower(power_grid);
	pv.SetPower(power_pv);
	hws.SetPower(power_hws);
}

}

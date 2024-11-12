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
#include <control/Input.hpp>
#include <nlohmann/json.hpp>
#include <configuration/ConfigurationSolarControl.hpp>
#include <mqtt/Client.hpp>
#include <websocket/SolarControl.hpp>

using namespace std;
using configuration::ConfigurationSolarControl;

using nlohmann::json;

namespace energy {

GlobalMeter * GlobalMeter::instance = 0;

GlobalMeter::GlobalMeter()
:grid("grid", "grid-excess"), pv("pv"), hws("hws"), peak("peak"), offpeak("offpeak")
{
	topic_em = ConfigurationSolarControl::GetInstance()->Get("energy.mqtt.id") + "/events/rpc";
	hws_min_energy = configuration::ConfigurationSolarControl::GetInstance()->GetInt("energy.hws.min");
	mqtt::Client::GetInstance()->Subscribe(topic_em, this);

	string offpeak_mqtt_id = ConfigurationSolarControl::GetInstance()->Get("offpeak.mqtt.id");
	if(offpeak_mqtt_id!="")
	{
		string ip = ConfigurationSolarControl::GetInstance()->Get("offpeak.ip");
		int input = ConfigurationSolarControl::GetInstance()->GetInt("offpeak.input");
		offpeak_ctrl = new control::Input(offpeak_mqtt_id, input, ip);
		offpeak_ctrl->UpdateState();
	}

	instance = this;
}

GlobalMeter::~GlobalMeter()
{
	if(offpeak_ctrl)
		delete offpeak_ctrl;

	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic_em, this);
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

double GlobalMeter::GetNetAvailablePower(bool allow_neg) const
{
	unique_lock<recursive_mutex> llock(lock);

	if(hws.GetEnergyConsumption()>hws_min_energy)
		return GetGrossAvailablePower(allow_neg);
	return GetExcessPower(allow_neg);
}

double GlobalMeter::GetGrossAvailablePower(bool allow_neg) const
{
	unique_lock<recursive_mutex> llock(lock);

	double available = hws.GetPower()-grid.GetPower();
	if(!allow_neg && available<0)
		return 0;
	return available;
}

double GlobalMeter::GetExcessPower(bool allow_neg) const
{
	unique_lock<recursive_mutex> llock(lock);

	double grid_power = grid.GetPower();
	if(!allow_neg && grid_power>0)
		return 0;
	return -grid_power;
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

bool GlobalMeter::GetOffPeak() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(!offpeak_ctrl)
		return false;

	return offpeak_ctrl->GetState();
}

bool GlobalMeter::GetOffPeakEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return peak.GetEnergyConsumption();
}

bool GlobalMeter::GetPeakEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return offpeak.GetEnergyConsumption();
}

void GlobalMeter::SaveHistory()
{
	grid.SaveHistory();
	pv.SaveHistory();
	hws.SaveHistory();
	peak.SaveHistory();
	offpeak.SaveHistory();
}

void GlobalMeter::HandleMessage(const string &message)
{
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

		if(offpeak_ctrl)
		{
			if(offpeak_ctrl->GetState())
				offpeak.SetPower(power_grid);
			else
				peak.SetPower(power_grid);
		}
	}

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::METER);
}


}

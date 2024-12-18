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
#include <device/DevicesOnOff.hpp>
#include <nlohmann/json.hpp>
#include <energy/ConfigurationEnergy.hpp>
#include <mqtt/Client.hpp>
#include <websocket/SolarControl.hpp>

using namespace std;
using configuration::ConfigurationEnergy;

using nlohmann::json;

namespace energy {

GlobalMeter * GlobalMeter::instance = 0;

GlobalMeter::GlobalMeter()
:grid(0, "grid", "grid-excess"), pv(0, "pv"), hws(0, "hws"), peak(0, "peak"), offpeak(0, "offpeak"), hws_forced(0, "hws-forced"), hws_offload(0, "hws-offload")
{
	Reload();

	instance = this;
}

GlobalMeter::~GlobalMeter()
{
	free();
}

void GlobalMeter::free()
{
	if(offpeak_ctrl)
	{
		delete offpeak_ctrl;
		offpeak_ctrl = 0;
	}

	if(meter_grid)
	{
		delete meter_grid;
		meter_grid = 0;
	}

	if(meter_pv)
	{
		delete meter_pv;
		meter_pv = 0;
	}

	if(meter_hws)
	{
		delete meter_hws;
		meter_hws = 0;
	}
}

void GlobalMeter::Reload()
{
	{
		unique_lock<recursive_mutex> llock(lock);

		free();

		auto config = ConfigurationEnergy::GetInstance();

		string mqtt_id = config->Get("energy.mqtt.id");
		string phase_grid = config->Get("energy.grid.phase");
		string phase_pv = config->Get("energy.pv.phase");
		string phase_hws = config->Get("energy.hws.phase");

		meter_grid = new meter::Pro3EM(mqtt_id, phase_grid);
		meter_pv = new meter::Pro3EM(mqtt_id, phase_pv);
		meter_hws = new meter::Pro3EM(mqtt_id, phase_hws);

		hws_min_energy = config->GetInt("energy.hws.min");

		string offpeak_mqtt_id = config->Get("offpeak.mqtt.id");
		if(offpeak_mqtt_id!="")
		{
			string ip = config->Get("offpeak.ip");
			int input = config->GetInt("offpeak.input");
			offpeak_ctrl = new control::Input(offpeak_mqtt_id, input, ip);
			offpeak_ctrl->UpdateState();
		}

		debug = config->GetBool("energy.debug.enabled");
		debug_grid = config->GetInt("energy.debug.grid");
		debug_pv= config->GetInt("energy.debug.pv");
		debug_hws = config->GetInt("energy.debug.hws");
	}

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::METER);
}

double GlobalMeter::GetGridPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(debug)
		return debug_grid;

	return meter_grid->GetPower();
}

double GlobalMeter::GetPVPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(debug)
		return debug_pv;

	double power = meter_pv->GetPower();
	return power>=0?power:0;
}

double GlobalMeter::GetHWSPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(debug)
		return debug_hws;

	return meter_hws->GetPower();
}

double GlobalMeter::GetPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	return GetGridPower() + GetPVPower();
}

double GlobalMeter::GetNetAvailablePower(bool allow_neg) const
{
	unique_lock<recursive_mutex> llock(lock);

	if(HWSIsFull())
		return GetGrossAvailablePower(allow_neg);
	return GetExcessPower(allow_neg);
}

double GlobalMeter::GetGrossAvailablePower(bool allow_neg) const
{
	unique_lock<recursive_mutex> llock(lock);

	double hws_offload = hws_state?0:GetHWSPower(); // No hws offload when in forced mode

	double available = hws_offload-GetGridPower();
	if(!allow_neg && available<0)
		return 0;
	return available;
}

double GlobalMeter::GetExcessPower(bool allow_neg) const
{
	unique_lock<recursive_mutex> llock(lock);

	double grid_power = GetGridPower();
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

	return offpeak.GetEnergyConsumption();
}

bool GlobalMeter::GetPeakEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return peak.GetEnergyConsumption();
}

void GlobalMeter::SetHWSState(bool new_state)
{
	unique_lock<recursive_mutex> llock(lock);

	hws_state = new_state;
}

bool GlobalMeter::HWSIsFull() const
{
	unique_lock<recursive_mutex> llock(lock);

	return hws.GetEnergyConsumption()>hws_min_energy;
}

void GlobalMeter::LogEnergy()
{
	{
		unique_lock<recursive_mutex> llock(lock);

		if(debug)
			return;

		double grid_consumption = meter_grid->GetConsumption();
		double grid_excess = meter_grid->GetExcess();
		double pv_production = meter_pv->GetConsumption();
		double hws_consumption = meter_hws->GetConsumption();

		grid.AddEnergy(grid_consumption, grid_excess);
		pv.AddEnergy(pv_production);
		hws.AddEnergy(hws_consumption);

		if(offpeak_ctrl)
		{
			if(offpeak_ctrl->GetState())
				offpeak.AddEnergy(grid_consumption);
			else
				peak.AddEnergy(grid_consumption);
		}

		if(hws_state)
			hws_forced.AddEnergy(hws_consumption);
		else
			hws_offload.AddEnergy(hws_consumption);
	}

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::METER);
}


}

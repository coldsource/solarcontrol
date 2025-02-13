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
#include <energy/ConfigurationEnergy.hpp>
#include <mqtt/Client.hpp>
#include <websocket/SolarControl.hpp>
#include <device/Devices.hpp>

using namespace std;
using configuration::ConfigurationEnergy;

using nlohmann::json;

namespace energy {

GlobalMeter * GlobalMeter::instance = 0;

GlobalMeter::GlobalMeter()
:grid(DEVICE_ID_GRID, "consumption", "excess"),
pv(DEVICE_ID_PV, "production"),
hws(DEVICE_ID_HWS, "consumption"),
hws_offload(DEVICE_ID_HWS, "offload")
{
	instance = this;

	Reload();
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

		hws_min_energy = config->GetEnergy("energy.hws.min");

		string offpeak_mqtt_id = config->Get("offpeak.mqtt.id");
		if(offpeak_mqtt_id!="")
		{
			string ip = config->Get("offpeak.ip");
			int input = config->GetInt("offpeak.input");
			offpeak_ctrl = new control::Input(offpeak_mqtt_id, input, ip);
			offpeak_ctrl->UpdateState();
		}

		debug = config->GetBool("energy.debug.enabled");
		debug_grid = config->GetPower("energy.debug.grid");
		debug_pv= config->GetPower("energy.debug.pv");
		debug_hws = config->GetPower("energy.debug.hws");
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

	double power = meter_hws->GetPower();
	return power>=0?power:0;
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

double GlobalMeter::GetPVPowerRatio() const
{
	unique_lock<recursive_mutex> llock(lock);

	double grid_power = GetGridPower();
	if(grid_power<=0)
		return 1;

	double pv_power = GetPVPower();
	double ratio = pv_power / (pv_power + grid_power);
	if(ratio>1)
		ratio = 1;
	return ratio;
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

double GlobalMeter::GetHWSOffloadEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return hws_offload.GetEnergyConsumption();
}

bool GlobalMeter::GetOffPeak() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(!offpeak_ctrl)
		return false;

	return offpeak_ctrl->GetState();
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

		double pv_ratio = GetPVPowerRatio();
		hws_offload.AddEnergy(hws_consumption * pv_ratio);
	}

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::METER);
}


}

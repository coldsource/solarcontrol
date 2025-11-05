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
#include <device/Devices.hpp>
#include <device/electrical/HWS.hpp>
#include <device/electrical/Grid.hpp>
#include <device/electrical/PV.hpp>
#include <device/electrical/Battery.hpp>
#include <websocket/SolarControl.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using device::Device;
using device::Devices;
using device::HWS;
using device::Grid;
using device::PV;
using device::Battery;
using nlohmann::json;

namespace energy {

GlobalMeter * GlobalMeter::instance = 0;

GlobalMeter::GlobalMeter()
{
	instance = this;

	hws = Devices::GetByID<HWS>(DEVICE_ID_HWS);
	grid = Devices::GetByID<Grid>(DEVICE_ID_GRID);
	pv = Devices::GetByID<PV>(DEVICE_ID_PV);
	battery = Devices::GetByID<Battery>(DEVICE_ID_BATTERY);

	// Register as configuration observer and trigger ConfigurationChanged() for initial config loading
	ObserveConfiguration("energy");
	ObserveConfiguration("control");
}

GlobalMeter::~GlobalMeter()
{
}

void GlobalMeter::ConfigurationChanged(const configuration::ConfigurationPart *config)
{
	if(config->GetType()=="energy")
		hws_min_energy = config->GetEnergy("energy.hws.min");

	if(config->GetType()=="control")
		hws_priority = (config->Get("control.priority")=="hws")?true:false;

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::METER);
}

bool GlobalMeter::HasBattery() const
{
	return battery->IsEnabled();
}

double GlobalMeter::GetBatteryVoltage() const
{
	return battery->GetVoltage();
}

double GlobalMeter::GetBatterySOC() const
{
	return battery->GetSOC();
}

double GlobalMeter::GetGridPower() const
{
	return grid->GetPower();
}

double GlobalMeter::GetPVPower() const
{
	double power = pv->GetPower();
	return power>=0?power:0;
}

double GlobalMeter::GetBatteryPower() const
{
	double power = battery->GetPower();
	return power>=0?power:0;
}

double GlobalMeter::GetHWSPower() const
{
	double power = hws->GetPower();
	return power>=0?power:0;
}

double GlobalMeter::GetPower() const
{
	return GetGridPower() + GetPVPower() + GetBatteryPower();
}

double GlobalMeter::GetNetAvailablePower(bool allow_neg) const
{
	if(HWSIsFull())
		return GetGrossAvailablePower(allow_neg);
	return GetExcessPower(allow_neg);
}

double GlobalMeter::GetGrossAvailablePower(bool allow_neg) const
{
	bool hws_state = hws->GetState();

	double hws_offload = hws_state?0:GetHWSPower(); // No hws offload when in forced mode

	double available = hws_offload-GetGridPower();
	if(!allow_neg && available<0)
		return 0;
	return available;
}

double GlobalMeter::GetExcessPower(bool allow_neg) const
{
	double grid_power = GetGridPower();
	if(!allow_neg && grid_power>0)
		return 0;
	return -grid_power;
}

double GlobalMeter::GetPVPowerRatio() const
{
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
	return grid->GetEnergyConsumption();
}

double GlobalMeter::GetExportedEnergy() const
{
	return grid->GetEnergyExcess();
}

double GlobalMeter::GetPVEnergy() const
{
	return pv->GetEnergyConsumption();
}

double GlobalMeter::GetBatteryEnergy() const
{
	return battery->GetEnergyConsumption();
}

double GlobalMeter::GetHWSEnergy() const
{
	return hws->GetEnergyConsumption();
}

double GlobalMeter::GetHWSOffloadEnergy() const
{
	return hws->GetEnergyOffload();
}

bool GlobalMeter::GetOffPeak() const
{
	return grid->GetOffPeak();
}

bool GlobalMeter::HWSIsFull() const
{
	if(!hws_priority)
		return true; // Offload priority, consider HWS is always full

	return hws->GetEnergyConsumption()>hws_min_energy;
}

json GlobalMeter::ToJson() const
{
	json j;
	j["has_battery"] = HasBattery();
	j["battery_voltage"] = GetBatteryVoltage();
	j["battery_soc"] = GetBatterySOC();

	j["grid"] = GetGridPower();
	j["pv"] = GetPVPower();
	j["battery"] = GetBatteryPower();
	j["hws"] = GetHWSPower();

	j["total"] = GetPower();
	j["net_available"] = GetNetAvailablePower();
	j["gross_available"] = GetGrossAvailablePower();
	j["excess"] = GetExcessPower();

	j["grid_energy"] = GetGridEnergy();
	j["grid_exported_energy"] = GetExportedEnergy();
	j["pv_energy"] = GetPVEnergy();
	j["battery_energy"] = GetBatteryEnergy();
	j["hws_energy"] = GetHWSEnergy();
	j["hws_energy_offload"] = GetHWSOffloadEnergy();

	j["offpeak"] = GetOffPeak();

	return j;
}

}

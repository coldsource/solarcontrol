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
#include <energy/ConfigurationEnergy.hpp>
#include <control/ConfigurationControl.hpp>
#include <device/Devices.hpp>
#include <device/electrical/DeviceHWS.hpp>
#include <device/electrical/DeviceGrid.hpp>
#include <device/electrical/DevicePV.hpp>
#include <device/electrical/DeviceBattery.hpp>
#include <websocket/SolarControl.hpp>

using namespace std;
using configuration::ConfigurationEnergy;
using configuration::ConfigurationControl;
using device::Device;
using device::DeviceHWS;
using device::DeviceGrid;
using device::DevicePV;
using device::DeviceBattery;

namespace energy {

GlobalMeter * GlobalMeter::instance = 0;

GlobalMeter::GlobalMeter()
{
	instance = this;

	ObserveDevice(DEVICE_ID_GRID);
	ObserveDevice(DEVICE_ID_PV);
	ObserveDevice(DEVICE_ID_HWS);
	ObserveDevice(DEVICE_ID_BATTERY);

	// Register as configuration observer and trigger ConfigurationChanged() for initial config loading
	auto config_energy = ConfigurationEnergy::GetInstance();
	ObserveConfiguration(config_energy);

	auto config_control = ConfigurationControl::GetInstance();
	ObserveConfiguration(config_control);
}

GlobalMeter::~GlobalMeter()
{
}

void GlobalMeter::ConfigurationChanged(const configuration::Configuration *config)
{
	{
		unique_lock<recursive_mutex> llock(lock);

		if(config->GetType()=="energy")
		{
			hws_min_energy = config->GetEnergy("energy.hws.min");

			debug = config->GetBool("energy.debug.enabled");
			debug_grid = config->GetPower("energy.debug.grid");
			debug_pv= config->GetPower("energy.debug.pv");
			debug_battery= config->GetPower("energy.debug.battery");
			debug_hws = config->GetPower("energy.debug.hws");
		}

		if(config->GetType()=="control")
			priority = config->Get("control.priority");
	}

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::METER);
}

void GlobalMeter::DeviceChanged(Device * device)
{
	unique_lock<recursive_mutex> llock(lock);

	if(device->GetID()==DEVICE_ID_HWS)
		hws = (DeviceHWS *)device;
	else if(device->GetID()==DEVICE_ID_GRID)
		grid = (DeviceGrid *)device;
	else if(device->GetID()==DEVICE_ID_PV)
		pv = (DevicePV *)device;
	else if(device->GetID()==DEVICE_ID_BATTERY)
		battery = (DeviceBattery *)device;
}

double GlobalMeter::GetGridPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(debug)
		return debug_grid;

	return grid->GetPower();
}

double GlobalMeter::GetPVPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(debug)
		return debug_pv;

	double power = pv->GetPower();
	return power>=0?power:0;
}

double GlobalMeter::GetBatteryPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(debug)
		return debug_battery;

	double power = battery->GetPower();
	return power>=0?power:0;
}

double GlobalMeter::GetHWSPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(debug)
		return debug_hws;

	double power = hws->GetPower();
	return power>=0?power:0;
}

double GlobalMeter::GetPower() const
{
	unique_lock<recursive_mutex> llock(lock);

	return GetGridPower() + GetPVPower() + GetBatteryPower();
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

	bool hws_state = hws->GetState();

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

	return grid->GetEnergyConsumption();
}

double GlobalMeter::GetExportedEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return grid->GetEnergyExcess();
}

double GlobalMeter::GetPVEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return pv->GetEnergyConsumption();
}

double GlobalMeter::GetBatteryEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return battery->GetEnergyConsumption();
}

double GlobalMeter::GetHWSEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return hws->GetEnergyConsumption();
}

double GlobalMeter::GetHWSOffloadEnergy() const
{
	unique_lock<recursive_mutex> llock(lock);

	return hws->GetEnergyOffload();
}

bool GlobalMeter::GetOffPeak() const
{
	unique_lock<recursive_mutex> llock(lock);

	return grid->GetOffPeak();
}

bool GlobalMeter::HWSIsFull() const
{
	unique_lock<recursive_mutex> llock(lock);

	if(priority=="offload")
		return true; // Offload priority, consider HWS is always full

	return hws->GetEnergyConsumption()>hws_min_energy;
}

}

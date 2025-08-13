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

#ifndef __ENERGY_GLOBALMETER_HPP__
#define __ENERGY_GLOBALMETER_HPP__

#include <device/DeviceObserver.hpp>
#include <configuration/ConfigurationObserver.hpp>

#include <mutex>
#include <string>
#include <memory>

namespace device {
	class Device;
	class DeviceHWS;
	class DeviceGrid;
	class DevicePV;
	class DeviceBattery;
}

namespace energy {

class GlobalMeter: public device::DeviceObserver, public configuration::ConfigurationObserver
{
	std::shared_ptr<device::DeviceGrid> grid;
	std::shared_ptr<device::DevicePV> pv;
	std::shared_ptr<device::DeviceHWS> hws;
	std::shared_ptr<device::DeviceBattery> battery;

	double hws_min_energy = 0;

	bool debug = false;
	double debug_grid = 0;
	double debug_pv = 0;
	double debug_battery = 0;
	double debug_hws = 0;

	std::string priority;

	static GlobalMeter *instance;
	mutable std::recursive_mutex lock;

protected:
	public:
		GlobalMeter();
		~GlobalMeter();

		static GlobalMeter *GetInstance() { return instance; }

		void ConfigurationChanged(const configuration::ConfigurationPart *config);
		void DeviceChanged(std::shared_ptr<device::Device> device);

		bool HasBattery() const;
		double GetBatteryVoltage() const;
		double GetBatterySOC() const;

		double GetGridPower() const; // Grid power (>0 if importing, <0 if exporting)
		double GetPVPower() const; // Solar production (>0 if producting)
		double GetBatteryPower() const; // Battery injection
		double GetHWSPower() const; // HWS consumption

		double GetPower() const; // Total consumption
		double GetNetAvailablePower(bool allow_neg = false) const; // Available power with HWS ON
		double GetGrossAvailablePower(bool allow_neg = false) const; // Available power excluding HWS consuption
		double GetExcessPower(bool allow_neg = false) const; // Power exported to grid
		double GetPVPowerRatio() const; // Percentage of solar power consumption

		double GetGridEnergy() const; // Energy taken from grid
		double GetExportedEnergy() const; // Total engergy exported to grid
		double GetPVEnergy() const; // Total produced energy
		double GetBatteryEnergy() const; // Total injected energy
		double GetHWSEnergy() const; // Total energy consummed by HWS
		double GetHWSOffloadEnergy() const; // Total energy consummed by HWS

		bool GetOffPeak() const;

		bool HWSIsFull() const;
};

}

#endif

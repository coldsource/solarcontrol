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

#include <energy/Counter.hpp>
#include <energy/Amount.hpp>
#include <meter/Pro3EM.hpp>

#include <mutex>
#include <map>

namespace energy {
	class Counter;
}

namespace control {
	class Input;
}

namespace energy {

class GlobalMeter
{
	meter::Pro3EM *meter_grid = 0;
	meter::Pro3EM *meter_pv = 0;
	meter::Pro3EM *meter_hws = 0;

	Counter grid;
	Counter pv;
	Counter hws;
	Counter hws_offload;

	control::Input *offpeak_ctrl = 0;

	double hws_min_energy = 0;
	bool hws_state = false;

	bool debug = false;
	double debug_grid = 0;
	double debug_pv = 0;
	double debug_hws = 0;

	static GlobalMeter *instance;
	mutable std::recursive_mutex lock;

	void free();

protected:
	public:
		GlobalMeter();
		~GlobalMeter();

		static GlobalMeter *GetInstance() { return instance; }

		void Reload();

		double GetGridPower() const; // Grid power (>0 if importing, <0 if exporting)
		double GetPVPower() const; // Solar production (>0 if producting)
		double GetHWSPower() const; // HWS consumption

		double GetPower() const; // Total consumption
		double GetNetAvailablePower(bool allow_neg = false) const; // Available power with HWS ON
		double GetGrossAvailablePower(bool allow_neg = false) const; // Available power excluding HWS consuption
		double GetExcessPower(bool allow_neg = false) const; // Power exported to grid
		double GetPVPowerRatio() const; // Percentage of solar power consumption

		double GetGridEnergy() const; // Energy taken from grid
		double GetExportedEnergy() const; // Total engergy exported to grid
		double GetPVEnergy() const; // Total produced energy
		double GetHWSEnergy() const; // Total energy consummed by HWS
		double GetHWSOffloadEnergy() const; // Total energy consummed by HWS

		bool GetOffPeak() const;

		double GetTotalHWSConsuptionForLast(int ndays) { return hws.GetTotalConsumptionForLast(ndays); }
		const std::map<datetime::Date, Amount> &GetGridConsumptionHistory() const { return grid.GetConsumptionHistory(); }
		const std::map<datetime::Date, Amount> &GetGridExcessHistory() const { return grid.GetExcessHistory(); }
		const std::map<datetime::Date, Amount> &GetPVProductionHistory() const { return pv.GetConsumptionHistory(); }
		const std::map<datetime::Date, Amount> &GetHWSConsumptionHistory() const { return hws.GetConsumptionHistory(); }
		const std::map<datetime::Date, Amount> &GetHWSOffloadConsumptionHistory() const { return hws_offload.GetConsumptionHistory(); }

		void SetHWSState(bool new_state);
		bool HWSIsFull() const;

		void LogEnergy();
};

}

#endif

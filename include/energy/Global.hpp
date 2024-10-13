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

#ifndef __GLOBAL_HPP__
#define __GLOBAL_HPP__

namespace energy {

class Counter;

class Global
{
	static Global *instance;

	Counter *grid;
	Counter *pv;
	Counter *hws;

	double hws_min_energy = 0;

	public:
		Global(Counter *grid, Counter *pv, Counter *hws);

		static Global *GetInstance() { return instance; }

		void SetHWSMinEnergy(double e) { hws_min_energy = e; }

		double GetGridPower() const; // Grid power (>0 if importing, <0 if exporting)
		double GetPVPower() const; // Solar production (>0 if producting)

		double GetPower() const; // Total consumption
		double GetNetAvailablePower() const; // Available power with HWS ON
		double GetGrossAvailablePower() const; // Available power excluding HWS consuption
		double GetExcessPower() const; // Power exported to grid

		double GetGridEnergy() const; // Energy taken from grid
		double GetExportedEnergy() const; // Total engergy exported to grid
		double GetPVEnergy() const; // Total produced energy
		double GetHWSEnergy() const; // Total energy consummed by HWS
};

}

#endif

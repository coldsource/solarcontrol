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

#include <energy/Global.hpp>
#include <energy/Counter.hpp>

namespace energy {

Global *Global::instance = 0;

Global::Global(Counter *grid, Counter *pv, Counter *hws)
{
	instance = this;

	this->grid = grid;
	this->pv = pv;
	this->hws = hws;
}

double Global::GetGridPower()
{
	return grid->GetPower();
}
double Global::GetPVPower()
{
	return pv->GetPower();
}

double Global::GetPower() const
{
	return grid->GetPower() + pv->GetPower();
}

double Global::GetNetAvailablePower() const
{
	if(hws->GetEnergyConsumption()>hws_min_energy)
		return GetGrossAvailablePower();
	return GetExcessPower();
}

double Global::GetGrossAvailablePower() const
{
	double available = hws->GetPower()-grid->GetPower();
	return (available<0)?0:available;
}

double Global::GetExcessPower() const
{
	double grid_power = grid->GetPower();

	return (grid_power>0)?0:-grid_power;
}

}

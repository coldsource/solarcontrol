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

#ifndef __ENERGY_AMOUNT_HPP__
#define __ENERGY_AMOUNT_HPP__

#include <nlohmann/json.hpp>

namespace energy {

class Amount
{
	protected:
		double energy = 0;
		double energy_peak = 0;
		double energy_offpeak = 0;

	public:
		Amount() {}
		Amount(double val);
		Amount(double val, double val_peak, double val_offpeak): energy(val), energy_peak(val_peak), energy_offpeak(val_offpeak) {}

		Amount operator+(const Amount &r) const;
		Amount &operator+=(const Amount &r);
		operator double() const;
		operator nlohmann::json() const;

		double GetEnergy() const { return energy; }
		double GetEnergyPeak() const { return energy_peak; }
		double GetEnergyOffPeak() const { return energy_offpeak; }
};

}

#endif



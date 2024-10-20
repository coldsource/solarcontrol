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

#ifndef __ENERGY_COUNTER_HPP__
#define __ENERGY_COUNTER_HPP__

namespace energy {

class Counter
{
	double last_ts;
	double last_yday;

	double power;
	double energy_consumption;
	double energy_excess;

	public:
		Counter();

		void SetPower(double v);
		double GetPower() const;
		double GetEnergyConsumption()  const;
		double GetEnergyExcess()  const;
};

}

#endif

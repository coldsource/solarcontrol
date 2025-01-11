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

#include <energy/Amount.hpp>
#include <energy/GlobalMeter.hpp>

using namespace std;
using namespace nlohmann;

namespace energy {

Amount::Amount(double val): energy(val)
{
	if(GlobalMeter::GetInstance()->GetOffPeak())
		energy_offpeak = val;
	else
		energy_peak = val;
}

Amount Amount::operator+(const Amount &r) const
{
	return Amount(energy + r.energy, energy_peak + r.energy_peak, energy_offpeak + r.energy_offpeak);
}

Amount &Amount::operator+=(const Amount &r)
{
	energy += r.energy;
	energy_peak += r.energy_peak;
	energy_offpeak += r.energy_offpeak;
	return *this;
}

Amount::operator double() const
{
	return energy;
}

Amount::operator nlohmann::json() const
{
	json jres;
	jres["energy"] = energy;
	jres["peak"] = energy_peak;
	jres["offpeak"] = energy_offpeak;

	return jres;
}

}

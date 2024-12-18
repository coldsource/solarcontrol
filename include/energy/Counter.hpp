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

#include <energy/HistoryDay.hpp>
#include <energy/HistoryQuarterHour.hpp>

#include <map>

namespace energy {

class Counter
{
	int last_yday;

	double energy_consumption;
	double energy_excess;

	HistoryDay consumption_history;
	HistoryDay excess_history;
	HistoryQuarterHour consumption_history_detail;
	HistoryQuarterHour excess_history_detail;

	public:
		Counter(unsigned int device_id = 0, const std::string &consumption_history_type = "", const std::string &excess_history_type = "");

		void AddEnergy(double consumption, double excess = 0);
		double GetEnergyConsumption()  const;
		double GetEnergyExcess()  const;

		double GetTotalConsumptionForLast(int ndays) const { return consumption_history.GetTotalForLast(ndays); }
		double GetTotalExcessForLast(int ndays) const { return excess_history.GetTotalForLast(ndays); }

		const std::map<datetime::Date, double> &GetConsumptionHistory() const { return consumption_history.Get(); }
		const std::map<datetime::Date, double> &GetExcessHistory() const { return excess_history.Get(); }
};

}

#endif

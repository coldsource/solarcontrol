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

#ifndef __ENERGY_HISTORYQUARTERHOUR_HPP__
#define __ENERGY_HISTORYQUARTERHOUR_HPP__

#include <stat/History.hpp>
#include <datetime/QuarterHour.hpp>
#include <energy/Amount.hpp>

#include <string>
#include <map>

namespace energy {

class HistoryQuarterHour: public stat::History<datetime::QuarterHour, Amount>
{
	std::string type;
	int device_id;

	protected:
		virtual void store_entry(const datetime::QuarterHour period, Amount value) override;
		virtual void save() override;

	public:
		HistoryQuarterHour(int device_id, const std::string &type = "");
		virtual ~HistoryQuarterHour();
};

}

#endif


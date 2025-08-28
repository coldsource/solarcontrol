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

#ifndef __HT_HISTORYQUARTERHOURHT_HPP__
#define __HT_HISTORYQUARTERHOURHT_HPP__

#include <stat/History.hpp>
#include <datetime/QuarterHour.hpp>
#include <weather/MinMaxHT.hpp>

namespace weather {

class HistoryQuarterHourHT: public stat::History<datetime::QuarterHour, weather::MinMaxHT>
{
	protected:
		unsigned int device_id;

		virtual void store_entry(const datetime::QuarterHour period, weather::MinMaxHT value) override;
		virtual void save() override;

	public:
		HistoryQuarterHourHT(unsigned int device_id);
		~HistoryQuarterHourHT();
};

}

#endif



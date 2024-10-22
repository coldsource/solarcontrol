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

#ifndef __DATETIME_TIMERANGE_HPP__
#define __DATETIME_TIMERANGE_HPP__

#include <datetime/HourMinuteSecond.hpp>
#include <nlohmann/json.hpp>

#include <set>

namespace datetime {

class TimeRange
{
	HourMinuteSecond start;
	HourMinuteSecond end;
	std::set<int> days_of_week;

	public:
		TimeRange(const HourMinuteSecond &start, const HourMinuteSecond &end, const std::set<int> &days_of_week = {});
		TimeRange(const nlohmann::json &j);

		bool IsActive() const;
};

}

#endif

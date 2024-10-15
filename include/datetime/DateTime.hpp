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

#ifndef __DATETIME_DATETIME_HPP__
#define __DATETIME_DATETIME_HPP__

#include <time.h>

#include <string>

namespace datetime {

class Timestamp;

class DateTime
{
	struct tm tm;

	public:
		DateTime();
		DateTime(const Timestamp &ts);

		int GetHour() { return tm.tm_hour; }
		int GetMinute() { return tm.tm_min; }
		int GetSecond() { return tm.tm_sec; }

		int GetWeekDay() { return tm.tm_wday; }
		int GetEUWeekDay() { return (tm.tm_wday + 6)%7; }
		int GetMonthDay() { return tm.tm_mday; }
		int GetYearDay() { return tm.tm_yday; }

		bool IsSameDay(const DateTime &dt) const ;
		void ToNoon();

		bool operator<(const DateTime& r) const;

		operator struct tm() const;
		operator std::string() const;
		operator Timestamp() const;
};

}

#endif


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
	protected:
		struct tm tm;

	public:
		DateTime();
		DateTime(const Timestamp &ts);

		int GetHour() const { return tm.tm_hour; }
		int GetMinute() const { return tm.tm_min; }
		int GetSecond() const { return tm.tm_sec; }

		int GetWeekDay() const { return tm.tm_wday; }
		int GetEUWeekDay() const { return (tm.tm_wday + 6)%7; }
		int GetMonthDay() const { return tm.tm_mday; }
		int GetMonth() const { return tm.tm_mon; }
		int GetYearDay() const { return tm.tm_yday; }

		bool IsSameDay(const DateTime &dt) const ;
		void ToNoon();

		bool operator<(const DateTime& r) const;

		operator struct tm() const;
		operator std::string() const;
		operator Timestamp() const;
};

}

#endif


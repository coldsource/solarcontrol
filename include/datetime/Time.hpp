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

#ifndef __TIME_HPP__
#define __TIME_HPP__

namespace datetime {

class Time
{
	int hour;
	int minute;
	int second;

	int wday;
	int mday;
	int yday;

	public:
		Time();

		int GetHour() { return hour; }
		int GetMinute() { return minute; }
		int GetSecond() { return second; }

		int GetWeekDay() { return wday; }
		int GetMonthDay() { return mday; }
		int GetYearDay() { return yday; }
};

}

#endif


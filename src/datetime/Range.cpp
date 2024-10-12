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

#include <datetime/Range.hpp>
#include <datetime/Time.hpp>

namespace datetime {

Range::Range(const HourMinuteSecond &start, const HourMinuteSecond &end, int day_of_week): start(start), end(end)
{
	this->day_of_week = day_of_week;
}

bool Range::IsActive() const
{
	Time t;
	if(day_of_week!=-1 && t.GetWeekDay()!=day_of_week)
		return false;

	bool after_start =
		t.GetHour()>start.GetHour()
		|| (t.GetHour()==start.GetHour() && t.GetMinute()>start.GetMinute())
		|| (t.GetHour()==start.GetHour() && t.GetMinute()==start.GetMinute() && t.GetSecond()>=start.GetSecond());

	bool before_end =
		t.GetHour()<end.GetHour()
		|| (t.GetHour()==end.GetHour() && t.GetMinute()<end.GetMinute())
		|| (t.GetHour()==end.GetHour() && t.GetMinute()==end.GetMinute() && t.GetSecond()<=end.GetSecond());

	return (after_start && before_end);
}

}

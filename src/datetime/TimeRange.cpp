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

#include <datetime/TimeRange.hpp>
#include <datetime/DateTime.hpp>
#include <energy/GlobalMeter.hpp>

#include <stdexcept>

using namespace std;
using configuration::Json;

namespace datetime {

TimeRange::TimeRange(const HourMinuteSecond &start, const HourMinuteSecond &end, const set<int> &days_of_week): start(start), end(end)
{
	this->days_of_week = days_of_week;
}

TimeRange::TimeRange(const Json &conf)
{
	start = HourMinuteSecond(conf.GetString("from"));
	end = HourMinuteSecond(conf.GetString("to"));

	if(conf.Has("offpeak"))
		offpeak = conf.GetBool("offpeak");

	if(conf.Has("days_of_week"))
	{
		for(auto day_of_week: conf.GetArray("days_of_week"))
			days_of_week.insert(day_of_week);
	}

	if(conf.Has("data"))
		data = conf.GetObject("data");
}

bool TimeRange::IsActive(configuration::Json *data_ptr) const
{
	if(offpeak && !energy::GlobalMeter::GetInstance()->GetOffPeak())
		return false;

	DateTime t;
	if(days_of_week.size()>0 && !days_of_week.contains(t.GetEUWeekDay()))
		return false;

	bool after_start =
		t.GetHour()>start.GetHour()
		|| (t.GetHour()==start.GetHour() && t.GetMinute()>start.GetMinute())
		|| (t.GetHour()==start.GetHour() && t.GetMinute()==start.GetMinute() && t.GetSecond()>=start.GetSecond());

	bool before_end =
		t.GetHour()<end.GetHour()
		|| (t.GetHour()==end.GetHour() && t.GetMinute()<end.GetMinute())
		|| (t.GetHour()==end.GetHour() && t.GetMinute()==end.GetMinute() && t.GetSecond()<=end.GetSecond());


	if(after_start && before_end)
	{
		if(data_ptr)
			*data_ptr = data;
		return true;
	}

	return false;
}

}

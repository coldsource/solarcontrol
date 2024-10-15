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

#include <datetime/DateTimeRange.hpp>
#include <datetime/Timestamp.hpp>
#include <datetime/DateTime.hpp>

#include <stdexcept>

using namespace std;

namespace datetime
{

DateTimeRange::DateTimeRange(const Timestamp &start, const Timestamp &end)
{
	start_t = start;
	end_t = end;

	if(start_t>end_t)
		throw logic_error("TimeRange start cannot be greater than end");
}

DateTimeRange::Iterator& DateTimeRange::Iterator::operator++()
{
	if(cur_t==dtr_ptr->end_t)
	{
		cur_t = dtr_ptr->end_t+1;
		return *this;
	}

	struct tm cur_tm;
	struct tm end_tm;
	localtime_r(&cur_t, &cur_tm);
	localtime_r(&dtr_ptr->end_t, &end_tm);

	if(cur_tm.tm_yday==end_tm.tm_yday && cur_tm.tm_year==end_tm.tm_year)
		cur_t = dtr_ptr->end_t;
	else
	{
		cur_tm.tm_mday++;
		cur_tm.tm_hour = 0;
		cur_tm.tm_min = 0;
		cur_tm.tm_sec = 0;
		cur_t = mktime(&cur_tm);
	}

	return *this;
}

bool DateTimeRange::Iterator::operator==(const Iterator& r) const
{
	return (cur_t==r.cur_t && dtr_ptr==r.dtr_ptr);
}

Timestamp DateTimeRange::Iterator::operator*() const
{
	return Timestamp(cur_t);
}

}

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

#include <datetime/DateTime.hpp>
#include <datetime/Timestamp.hpp>

#include <time.h>
#include <string.h>

using namespace std;

namespace datetime {

const regex DateTime::ymdhms("^([0-9]{4,4})-([0-9]{2,2})-([0-9]{2,2}) ([0-9]{2,2}):([0-9]{2,2}):([0-9]{2,2})$");

DateTime::DateTime()
{
	time_t t = time(0);
	localtime_r(&t, &tm);
}

DateTime::DateTime(const Timestamp &ts)
{
	time_t t = ts;
	localtime_r(&t, &tm);
}

DateTime::DateTime(const string &str)
{
	smatch matches;

	if(!regex_search(str, matches, ymdhms))
		throw invalid_argument("Invalid date time format : « " + str + " »");

	// First init tm structure with local time zone
	time_t t = time(0);
	localtime_r(&t, &tm);

	strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
}

bool DateTime::IsSameDay(const DateTime &dt) const
{
	return (tm.tm_year==dt.tm.tm_year && tm.tm_yday==dt.tm.tm_yday);

}
void DateTime::ToNoon()
{
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;
}

bool DateTime::operator==(const DateTime& r) const
{
	time_t l_t = (time_t)(Timestamp)*this;
	time_t r_t = (time_t)(Timestamp)r;
	return l_t==r_t;
}

bool DateTime::operator<(const DateTime& r) const
{
	time_t l_t = (time_t)(Timestamp)*this;
	time_t r_t = (time_t)(Timestamp)r;
	return l_t<r_t;
}

DateTime::operator struct tm() const
{
	return tm;
}

DateTime::operator string() const
{
	char str[32];
	size_t s = strftime(str, 32, "%Y-%m-%d %H:%M:%S", &tm);
	return string(str, s);
}

DateTime::operator Timestamp() const
{
	struct tm ltm = tm;
	time_t ts = mktime(&ltm);
	return Timestamp(ts);
}

}

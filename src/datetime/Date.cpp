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

#include <datetime/Date.hpp>
#include <datetime/Timestamp.hpp>

#include <regex>
#include <stdexcept>

using namespace std;

namespace datetime {

const regex Date::ymd("^([0-9]{4,4})-([0-9]{2,2})-([0-9]{2,2})$");

Date::Date(): DateTime()
{
	ToNoon();
}

Date::Date(const Timestamp &ts): DateTime(ts)
{
	ToNoon();
}

Date::Date(const string &str)
{
	smatch matches;

	if(!regex_search(str, matches, ymd))
		throw invalid_argument("Invalid date format : « " + str + " »");

	// First init tm structure with local time zone
	time_t t = time(0);
	localtime_r(&t, &tm);

	strptime(str.c_str(), "%Y-%m-%d", &tm);

	ToNoon();
}

Date Date::operator-(int days) const
{
	Date before_d(*this);

	before_d.tm.tm_mday -= days;
	mktime(&before_d.tm);

	return before_d;
}

Date Date::operator+(int days) const
{
	Date after_d(*this);

	after_d.tm.tm_mday += days;
	mktime(&after_d.tm);

	return after_d;
}

Date::operator string() const
{
	char str[32];
	size_t s = strftime(str, 32, "%Y-%m-%d", &tm);
	return string(str, s);
}

}

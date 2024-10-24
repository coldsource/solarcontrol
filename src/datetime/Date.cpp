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

#include <string.h>

#include <regex>
#include <stdexcept>

using namespace std;

namespace datetime {

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
	regex hms("^([0-9]{4,4})-([0-9]{2,2})-([0-9]{2,2})$");
	smatch matches;

	if(!regex_search(str, matches, hms))
		throw invalid_argument("Invalid date format : « " + str + " »");

	memset(&tm, 0, sizeof(tm));
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

Date::operator string() const
{
	char str[32];
	size_t s = strftime(str, 32, "%Y-%m-%d", &tm);
	return string(str, s);
}

}

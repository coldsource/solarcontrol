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
#include <stdio.h>

using namespace std;

namespace datetime {

DateTime::DateTime()
{
	time_t t = time(NULL);
	localtime_r(&t, &tm);
}

DateTime::DateTime(const Timestamp &ts)
{
	time_t t = ts;
	localtime_r(&t, &tm);
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

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

#include <datetime/Timestamp.hpp>
#include <datetime/DateTime.hpp>

#include <time.h>

#include <stdexcept>

using namespace std;

namespace datetime {

Timestamp::Timestamp()
{
	ts_i = 0;
	ts_d = 0;
}

Timestamp::Timestamp(int type)
{
	struct timespec t;

	if(type==TS_MONOTONIC)
		clock_gettime(CLOCK_MONOTONIC, &t);
	else if(type==TS_REAL)
		clock_gettime(CLOCK_REALTIME, &t);
	else
		throw invalid_argument("Unknown timestamp type");

	ts_i = t.tv_sec;
	ts_d = (double)t.tv_sec +  (double)t.tv_nsec / 1000000000;
}

Timestamp::Timestamp(time_t ts)
{
	ts_i = ts;
	ts_d = (double)ts;
}

Timestamp& Timestamp::operator+=(time_t t_inc)
{
	ts_i += t_inc;
	ts_d += (double)t_inc;

	return *this;
}

long Timestamp::operator-(const Timestamp &rts) const
{
	time_t r_t = (time_t)rts;
	if(r_t>ts_i)
		throw logic_error("Could not substract greater timestamp");

	return ts_i-r_t;
}

Timestamp::operator time_t() const
{
	return ts_i;
}

Timestamp::operator double() const
{
	return ts_d;
}

Timestamp::operator DateTime() const
{
	return DateTime(*this);
}

}

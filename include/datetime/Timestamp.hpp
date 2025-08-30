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

#ifndef __DATETIME_TIMESTAMP_HPP__
#define __DATETIME_TIMESTAMP_HPP__

#include <time.h>

#define TS_MONOTONIC        1
#define TS_REAL             2

namespace datetime {

class DateTime;

class Timestamp
{
	time_t ts_i;
	double ts_d;

	public:
		Timestamp();
		Timestamp(int type);
		Timestamp(time_t ts);

		bool Isempty() { return ts_i==0; }

		Timestamp& operator+=(time_t t_inc);
		long operator-(const Timestamp &rts) const;
		bool operator<(const Timestamp& r) const { return ts_i < r.ts_i; }

		operator time_t() const;
		operator double() const;
		operator DateTime() const;
};

}

#endif


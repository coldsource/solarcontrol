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

#include <time.h>

#include <stdexcept>

using namespace std;

namespace datetime {

Timestamp::Timestamp(int type)
{
	struct timespec t;

	if(type==TS_MONOTONIC)
		clock_gettime(CLOCK_MONOTONIC, &t);
	else if(type==TS_REAL)
		clock_gettime(CLOCK_REALTIME, &t);
	else
		throw invalid_argument("Unknown timestamp type");

	ts = (double)t.tv_sec +  (double)t.tv_nsec / 1000000000;
}

Timestamp::operator double() const
{
	return ts;
}

}

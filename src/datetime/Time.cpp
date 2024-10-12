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

#include <datetime/Time.hpp>

#include <time.h>
#include <stdio.h>

namespace datetime {

Time::Time()
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	hour = tm.tm_hour;
	minute = tm.tm_min;
	second = tm.tm_sec;

	wday = (tm.tm_wday + 6)%7;
	mday = tm.tm_mday;
	yday = tm.tm_yday;
}

}

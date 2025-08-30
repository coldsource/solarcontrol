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

#ifndef __DATETIME_TIMESPANHISTORY_HPP__
#define __DATETIME_TIMESPANHISTORY_HPP__

#include <time.h>

#include <vector>

namespace datetime {

class Timestamp;

class TimespanHistory
{
	struct st_timespan
	{
		time_t from = 0;
		time_t to = 0;
	};

	unsigned int retention_days;
	std::vector<st_timespan> history;

	void purge();

	public:
		TimespanHistory(int device_id = 0);

		void ClockIn();
		void ClockIn(const Timestamp &ts);
		void ClockOut();
		void ClockOut(const Timestamp &ts);

		unsigned long GetTotalForLast(unsigned int nseconds) const;
};

}

#endif

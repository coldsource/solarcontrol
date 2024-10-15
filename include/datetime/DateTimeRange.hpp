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

#ifndef __DATETIME_DATETIMERANGE_HPP__
#define __DATETIME_DATETIMERANGE_HPP__

#include <time.h>

namespace datetime {

class Timestamp;

class DateTimeRange
{
	time_t start_t;
	time_t end_t;

	public:
		DateTimeRange(const Timestamp &start, const Timestamp &end);

		// Iterator
		struct Iterator
		{
			private:
				DateTimeRange *dtr_ptr;
				time_t cur_t;

			public:
				Iterator(DateTimeRange *ptr, time_t cur_t): dtr_ptr(ptr), cur_t(cur_t) { }

				Iterator& operator++();
				bool operator==(const Iterator& r) const;
				Timestamp operator*() const;
		};

		Iterator begin()
		{
			return Iterator(this, start_t);
		}

		Iterator end()
		{
			return Iterator(this, end_t+1);
		}
};

}

#endif


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

#ifndef __THREAD_HISTORYSYNC_HPP__
#define __THREAD_HISTORYSYNC_HPP__

#include <thread/WaiterThread.hpp>

namespace stat {
	class HistorySync;
}

#include <set>
#include <mutex>

namespace thread {

class HistorySync: public WaiterThread
{
	static HistorySync *instance;

	std::mutex lock;

	int sync_interval;
	std::set<stat::HistorySync *> clients;

	void main(void);

	public:
		HistorySync();
		~HistorySync();

		static HistorySync *GetInstance() { return instance; }

		void Register(stat::HistorySync *client);
		void Unregister(stat::HistorySync *client);
};

}

#endif



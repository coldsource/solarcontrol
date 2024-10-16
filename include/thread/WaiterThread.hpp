/*
 * This file is part of evQueue
 *
 * evQueue is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * evQueue is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with evQueue. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Thibault Kummer <bob@coldsource.net>
 */

#ifndef __THREAD_WAITERTHREAD_H__
#define __THREAD_WAITERTHREAD_H__

#include <thread>
#include <mutex>
#include <condition_variable>

namespace thread {

class WaiterThread
{
	std::thread thread_handle;

	std::mutex wait_lock;
	std::condition_variable shutdown_requested;

	bool enabled = true;
	bool is_shutting_down = false;

	protected:
		void start();
		bool wait(int seconds);
		void set_enabled(bool enabled) { this->enabled = enabled; }

		static void thread_main(WaiterThread *ptr);
		virtual void main(void) = 0;

	public:
		void Shutdown(void);
		void WaitForShutdown(void);
};

}

#endif

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

#include <thread/WaiterThread.hpp>
#include <logs/Logger.hpp>

using namespace std;

namespace thread {

void WaiterThread::start()
{
	if(!enabled)
		return;

	thread_handle = std::thread(thread_main, this);
}

bool WaiterThread::wait(int seconds)
{
	while(true)
	{
		unique_lock<mutex> llock(wait_lock);

		cv_status ret;
		if(!is_shutting_down)
			ret = shutdown_requested.wait_for(llock, chrono::seconds(seconds));

		llock.unlock();

		if(is_shutting_down)
			return false;

		if(ret==cv_status::timeout)
			return true;

		// Suprious interrupt, continue waiting
	}
}

void WaiterThread::thread_main(WaiterThread *ptr)
{
	try
	{
		ptr->main();
	}
	catch(exception &e)
	{
		logs::Logger::Log(LOG_ERR, e.what());
	}
}

void WaiterThread::Shutdown(void)
{
	if(!enabled)
		return;

	if(thread_handle.get_id()==std::thread::id())
		return; // Thread not yet started, nothing to do

	wait_lock.lock();
	is_shutting_down = true;
	shutdown_requested.notify_one();
	wait_lock.unlock();
}

void WaiterThread::WaitForShutdown(void)
{
	if(!enabled)
		return;

	thread_handle.join();
}

}

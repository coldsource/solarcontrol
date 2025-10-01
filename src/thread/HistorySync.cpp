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

#include <thread/HistorySync.hpp>
#include <stat/History.hpp>
#include <configuration/Configuration.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <excpt/Database.hpp>
#include <logs/Logger.hpp>

using namespace std;

namespace thread {

HistorySync *HistorySync::instance = 0;

HistorySync::HistorySync()
{
	sync_interval = configuration::Configuration::FromType("solarcontrol")->GetTime("core.history.sync");

	start();

	instance = this;
}

HistorySync::~HistorySync()
{
	Shutdown();
	WaitForShutdown();
}

void HistorySync::Register(stat::HistorySync *client)
{
	unique_lock<mutex> llock(lock);

	clients.insert(client);
}
void HistorySync::Unregister(stat::HistorySync *client)
{
	unique_lock<mutex> llock(lock);

	clients.erase(client);
}

void HistorySync::main()
{
	while(true)
	{
		if(!wait(sync_interval))
			return;

		{
			unique_lock<mutex> llock(lock);

			for(auto client : clients)
			{
				try
				{
					client->Sync();
				}
				catch(excpt::Database &e)
				{
					e.Log(LOG_ERR);
				}
			}
		}
	}
}

}

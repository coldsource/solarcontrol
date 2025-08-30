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

#include <thread/SensorsManager.hpp>
#include <datetime/Timestamp.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <logs/Logger.hpp>

#include <stdexcept>

using namespace std;
using datetime::Timestamp;

namespace thread {

SensorsManager *SensorsManager::instance = 0;

SensorsManager::SensorsManager()
{
	// Register as configuration observer and trigger ConfigurationChanged() for initial config loading
	ObserveConfiguration("control");

	instance = this;
}

SensorsManager::~SensorsManager()
{
}

void SensorsManager::Register(shared_ptr<sensor::Sensor> sensor)
{
	unique_lock<mutex> llock(lock);

	new_sensors.insert(sensor);
	sensors.insert(sensor);

	Signal(); // Signal thread for immediate update
}

void SensorsManager::Unregister(shared_ptr<sensor::Sensor> sensor)
{
	unique_lock<mutex> llock(lock);

	sensors.erase(sensor);
}

void SensorsManager::ConfigurationChanged(const configuration::ConfigurationPart *config)
{
	unique_lock<mutex> llock(lock);

	state_update_interval = config->GetTime("control.state.update_interval");
	Signal(); // Signal in case our interval has changed
}

void SensorsManager::main()
{
	Timestamp last_state_update(TS_MONOTONIC);

	while(true)
	{
		if(!wait(1, [this]() { return new_sensors.size()==0; } )) // Wait 1 second unless we have new devices to update
			return;

		try
		{
			if(new_sensors.size()>0)
			{
				std::set<std::shared_ptr<sensor::Sensor>> local_sensors;

				{
					// Lock to copy all shared pointers
					unique_lock<mutex> llock(lock);

					local_sensors = new_sensors;
					new_sensors.clear();
				}

				// Call updated unlocked
				for(auto sensor : local_sensors)
					sensor->ForceUpdate();
			}

			Timestamp now(TS_MONOTONIC);
			if(now-last_state_update>state_update_interval)
			{
				std::set<std::shared_ptr<sensor::Sensor>> local_sensors;

				{
					// Lock to copy all shared pointers
					unique_lock<mutex> llock(lock);

					local_sensors = sensors;
				}

				// Call updated unlocked
				for(auto sensor : local_sensors)
					sensor->ForceUpdate();

				last_state_update = now;
			}
		}
		catch(exception &e)
		{
			logs::Logger::Log(LOG_ERR, "Error in sensors manager : « " + string(e.what()) + " »");
		}
	}
}

}


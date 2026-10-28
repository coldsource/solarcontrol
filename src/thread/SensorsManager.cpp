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
#include <device/Devices.hpp>
#include <device/Device.hpp>
#include <logs/Logger.hpp>
#include <excpt/Shelly.hpp>

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

void SensorsManager::ConfigurationChanged(const configuration::ConfigurationPart *config)
{
	unique_lock<mutex> llock(lock);

	state_update_interval = config->GetTime("control.state.update_interval");
	Signal(); // Signal in case our interval has changed
}

void SensorsManager::main()
{
	while(true)
	{
		try
		{
			auto devices = device::Devices::Get<device::Device>();

			for(auto device : devices)
			{
				device->SensorsForceUpdate();
			}
		}
		catch(exception &e)
		{
			logs::Logger::Log(LOG_ERR, "Error in sensors manager : « " + string(e.what()) + " »");
		}

		if(!wait(state_update_interval))
			return;
	}
}

}


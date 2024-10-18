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

#include <thread/DevicesManager.hpp>
#include <datetime/Timestamp.hpp>
#include <device/Devices.hpp>
#include <device/DevicesOnOff.hpp>
#include <device/DeviceOnOff.hpp>
#include <energy/GlobalMeter.hpp>
#include <websocket/SolarControl.hpp>
#include <configuration/ConfigurationSolarControl.hpp>
#include <logs/Logger.hpp>

#include <stdexcept>

using namespace std;

namespace thread {

DevicesManager *DevicesManager::instance = 0;

DevicesManager::DevicesManager()
{
	start();

	instance = this;
}

void DevicesManager::main()
{
	auto global = energy::GlobalMeter::GetInstance();

	device::DevicesOnOff &devices = device::Devices::GetInstance()->GetOnOff();

	auto config = configuration::ConfigurationSolarControl::GetInstance();
	unsigned long long state_update_interval = config->GetInt("control.state.update_interval");

	double start_cooldown = config->GetInt("control.cooldown.on");
	double last_start_ts = 0;
	datetime::Timestamp last_state_update(TS_MONOTONIC);
	while(true)
	{
		devices.Lock();

		try
		{
			datetime::Timestamp now(TS_MONOTONIC);
			for(auto it = devices.begin(); it!=devices.end(); ++it)
			{
				device::DeviceOnOff *device = *it;

				if(now-last_state_update>state_update_interval)
				{
					device->UpdateState();
					last_state_update = now;
				}

				bool new_state = device->WantedState();
				if(new_state==device->GetState())
					continue;

				if(new_state && (double)now-last_start_ts<start_cooldown)
					continue;

				device->SetState(new_state);
				if(new_state)
					last_start_ts = now;
			}
		}
		catch(exception &e)
		{
			logs::Logger::Log(LOG_ERR, e.what());
		}

		devices.Unlock();

		if(websocket::SolarControl::GetInstance())
			websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::METER);

		printf("%f %f %f\n", global->GetPower(), global->GetGrossAvailablePower(), global->GetNetAvailablePower());
		if(!wait(1))
			return;
	}
}

}

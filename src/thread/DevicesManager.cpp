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
#include <energy/Global.hpp>

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
	auto global = energy::Global::GetInstance();

	device::Devices devices;

	double start_cooldown = 10;
	double last_start_ts = 0;
	while(true)
	{
		devices.Lock();

		try
		{
			for(auto it = devices.begin(); it!=devices.end(); ++it)
			{
				device::Device *device = *it;

				bool new_state = device->WantedState();
				if(new_state==device->GetState())
					continue;

				double ts = datetime::Timestamp(TS_MONOTONIC);
				if(new_state && ts-last_start_ts<start_cooldown)
					continue;

				device->SetState(new_state);
				if(new_state)
					last_start_ts = ts;
			}
		}
		catch(exception &e)
		{
			fprintf(stderr, "%s\n", e.what());
		}

		devices.Unlock();

		printf("%f %f %f\n", global->GetPower(), global->GetGrossAvailablePower(), global->GetNetAvailablePower());
		if(!wait(1))
			return;
	}
}

}

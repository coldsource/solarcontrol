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

#ifndef __THREAD_DEVICESMANAGER_HPP__
#define __THREAD_DEVICESMANAGER_HPP__

#include <thread/WaiterThread.hpp>
#include <energy/MovingAverage.hpp>

#include <vector>
#include <map>

namespace energy {
	class GlobalMeter;
}

namespace device {
	class DeviceOnOff;
}

namespace thread {

class DevicesManager: public WaiterThread
{
	static DevicesManager *instance;

	protected:
		energy::GlobalMeter *global_meter;
		energy::MovingAverage available_power_avg;

		int hysteresis_export = 0;
		int hysteresis_import = 0;
		unsigned long long state_update_interval;
		double cooldown;

		bool hysteresis(double available_power, const device::DeviceOnOff *device) const;
		bool force(const std::map<device::DeviceOnOff *, bool> &devices);
		bool offload(const std::vector<device::DeviceOnOff *> &devices);

		void main(void);

	public:
		DevicesManager();

		static DevicesManager *GetInstance() { return instance; }
};

}

#endif


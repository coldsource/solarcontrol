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
#include <configuration/ConfigurationObserver.hpp>

#include <vector>
#include <map>
#include <mutex>
#include <memory>

namespace energy {
	class GlobalMeter;
}

namespace stat {
	class MovingAverage;
}

namespace device {
	class DeviceOnOff;
}

namespace thread {

class DevicesManager: public WaiterThread, public configuration::ConfigurationObserver
{
	static DevicesManager *instance;

	std::mutex lock;

	protected:
		energy::GlobalMeter *global_meter = 0;
		std::unique_ptr<stat::MovingAverage> available_power_avg; // Average available power, used to switch devices off
		std::unique_ptr<stat::MovingAverage> available_power_histo; // History of available power, used to switch devices on

		int hysteresis_export = 0;
		int hysteresis_import = 0;
		int hysteresis_precision = 0;
		unsigned long cooldown;

		bool hysteresis(double power_delta, const std::shared_ptr<device::DeviceOnOff> device) const;
		bool force(const std::map<std::shared_ptr<device::DeviceOnOff>, bool> &devices);
		bool offload(const std::vector<std::shared_ptr<device::DeviceOnOff>> &devices);

		void main(void) override;

	public:
		DevicesManager();
		virtual ~DevicesManager();

		static DevicesManager *GetInstance() { return instance; }

		void Start() { start(); }

		void ConfigurationChanged(const configuration::ConfigurationPart *config) override;
};

}

#endif


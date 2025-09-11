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

namespace device {
	class DeviceOnOff;
}


namespace thread {

class Stats;

class DevicesManager: public WaiterThread, public configuration::ConfigurationObserver
{
	static DevicesManager *instance;

	std::mutex lock;

	protected:
		energy::GlobalMeter *global_meter = 0;
		Stats *stats = 0;

		// Config
		int hysteresis_export = 0;
		int hysteresis_import = 0;
		double hysteresis_precision = 0;
		unsigned long cooldown;

		// State
		double controllable_power = 0;
		double forced_power = 0;
		double offloaded_power = 0;

		bool hysteresis(const std::shared_ptr<device::DeviceOnOff> device) const;
		bool force(const std::map<std::shared_ptr<device::DeviceOnOff>, bool> &devices);
		bool offload(const std::vector<std::shared_ptr<device::DeviceOnOff>> &devices);

		double get_controllable_power() const;

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


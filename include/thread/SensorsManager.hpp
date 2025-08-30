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

#ifndef __THREAD_SENSORSMANAGER_HPP__
#define __THREAD_SENSORSMANAGER_HPP__

#include <thread/WaiterThread.hpp>
#include <configuration/ConfigurationObserver.hpp>
#include <sensor/Sensor.hpp>

#include <mutex>
#include <memory>
#include <set>

namespace thread {

class SensorsManager: public WaiterThread, public configuration::ConfigurationObserver
{
	static SensorsManager *instance;

	std::set<std::shared_ptr<sensor::Sensor>> sensors;
	std::set<std::shared_ptr<sensor::Sensor>> new_sensors;

	unsigned long state_update_interval;

	std::mutex lock;

	protected:
		void main(void) override;

	public:
		SensorsManager();
		virtual ~SensorsManager();

		static SensorsManager *GetInstance() { return instance; }

		void Start() { start(); }

		void Register(std::shared_ptr<sensor::Sensor> sensor);
		void Unregister(std::shared_ptr<sensor::Sensor> sensor);

		void ConfigurationChanged(const configuration::ConfigurationPart *config) override;
};

}

#endif



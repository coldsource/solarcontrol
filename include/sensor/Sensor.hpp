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

#ifndef __SENSOR_SENSOR_HPP__
#define __SENSOR_SENSOR_HPP__

#include <string>
#include <atomic>
#include <mutex>
#include <memory>

namespace sensor {

class SensorObserver;

class Sensor: public std::enable_shared_from_this<Sensor>
{
	std::string name = "";

	mutable std::mutex lock;

	mutable std::mutex observer_lock;
	SensorObserver *observer = 0;

	protected:
		void notify_observer();

	public:
		Sensor():name() {}

		void SetObserver(SensorObserver * observer);
		void SetObserver();

		std::string GetName() const { return name; }
		void SetName(const std::string &name) { this->name = name; }

		virtual void ForceUpdate() {}

		virtual std::string GetCategory() const = 0;
};

}

#endif

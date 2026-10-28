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

#ifndef __SENSOR_SENSORS_HPP__
#define __SENSOR_SENSORS_HPP__

#include <sensor/Sensor.hpp>
#include <configuration/Json.hpp>

#include <string>
#include <set>
#include <memory>

namespace configuration {
	class Json;
}

namespace sensor {

class Sensor;
class SensorObserver;

class Sensors
{
	std::set<std::shared_ptr<Sensor>> sensors;

	SensorObserver *observer;

	public:
		Sensors(SensorObserver *observer):observer(observer) {}
		virtual ~Sensors();

		std::set<std::shared_ptr<Sensor>> Get() { return sensors; }

		void insert(std::shared_ptr<Sensor> sensor_ptr);
		void clear();
		bool has(const std::string &category) const;
};

}

#endif


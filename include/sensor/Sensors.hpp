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
#include <mqtt/Message.hpp>

#include <string>
#include <map>
#include <memory>

namespace device {
	class Device;
}

namespace sensor {

class Sensor;

class Sensors
{
	friend class device::Device;

	std::map<unsigned int, std::shared_ptr<Sensor>> sensors;

	device::Device *observer;

	public:
		Sensors(device::Device *observer):observer(observer) {}
		virtual ~Sensors();

	protected:
		auto get() { return sensors; }

		void insert(std::shared_ptr<Sensor> sensor_ptr);
		void clear();
		bool has(const std::string &category) const;

		std::pair<std::shared_ptr<Sensor>, bool> dispatch_mqtt_message(const mqtt::st_message &mqtt_msg);
};

}

#endif


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

#include <sensor/MQTTSubscriber.hpp>

#include <string>
#include <atomic>
#include <vector>
#include <memory>

namespace sensor {

class Factory;

class Sensor
{
	friend class Factory;

	inline static std::atomic<unsigned int> id_sequence{1};

	protected:
		unsigned int id;
		std::string name = "";

		std::vector<std::string> mqtt_topics;
		std::vector<std::shared_ptr<MQTTSubscriber>> mqtt_subscribers;

	public:
		Sensor(const std::string &name):id(id_sequence.fetch_add(1, std::memory_order_relaxed)), name(name) {}
		virtual ~Sensor();

		unsigned int GetID() { return id; }
		std::string GetName() const { return name; }

		virtual bool ForceUpdate() { return false; } // Will be called UNLOCKED and MUST be thread safe
		virtual bool HandleMQTTMessage(const std::string & /*message*/, const std::string & /*topic*/) { return false; };

		virtual std::string GetCategory() const = 0;
};

}

#endif

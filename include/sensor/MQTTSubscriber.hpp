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

#ifndef __SENSOR_MQTTSUBSCRIBER_HPP__
#define __SENSOR_MQTTSUBSCRIBER_HPP__

#include <mqtt/Subscriber.hpp>

#include <string>
#include <memory>

namespace sensor {

class MQTTSubscriber: public mqtt::Subscriber
{
	std::string topic;
	int device_id;
	unsigned int sensor_id;

	public:
		MQTTSubscriber(const std::string &topic, int device_id, unsigned int sensor_id):topic(topic), device_id(device_id), sensor_id(sensor_id) {}

		virtual std::shared_ptr<mqtt::Dispatcher> Get() const override;

		const std::string &GetTopic() const { return topic; }
		unsigned int GetSensorID() const { return sensor_id; }
};

}

#endif


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

#include <sensor/Sensors.hpp>
#include <sensor/Sensor.hpp>
#include <sensor/MQTTSubscriber.hpp>

using namespace std;

namespace sensor {

Sensors::~Sensors()
{
	clear();
}

void Sensors::insert(shared_ptr<Sensor> sensor_ptr)
{
	sensors[sensor_ptr->GetID()] = sensor_ptr;
}

void Sensors::clear()
{
	sensors.clear();
}

bool Sensors::has(const std::string &category) const
{
	for(const auto &sensor : sensors)
	{
		if(sensor.second->GetCategory()==category)
			return true;
	}

	return false;
}

pair<shared_ptr<Sensor>, bool> Sensors::dispatch_mqtt_message(const mqtt::st_message &mqtt_msg)
{
	auto subscriber = std::dynamic_pointer_cast<MQTTSubscriber>(mqtt_msg.subscriber);

	auto it = sensors.find(subscriber->GetSensorID());
	if(it==sensors.end())
		return {nullptr, false}; // Sensor not found, message is probably outdated

	auto sensor = it->second;
	bool has_changed = sensor->HandleMQTTMessage(mqtt_msg.message, mqtt_msg.topic);
	return {sensor, has_changed};
}

}

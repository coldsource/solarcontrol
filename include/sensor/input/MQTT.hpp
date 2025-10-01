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

#ifndef __INPUT_WEATHER_MQTT_HPP__
#define __INPUT_WEATHER_MQTT_HPP__

#include <sensor/input/Input.hpp>
#include <mqtt/Subscriber.hpp>

#include <string>
#include <atomic>

namespace configuration {
	class Json;
}

namespace sensor::input {

class MQTT: public Input, public mqtt::Subscriber
{
	const int input = 0;
	const std::string topic;
	const std::string ip;

	std::atomic_bool state = false;

	protected:
		bool get_input() const;

	public:
		MQTT(const std::string &mqtt_id, int input, const std::string &ip = "");
		virtual ~MQTT();

		static void CheckConfig(const configuration::Json &conf);

		virtual bool GetState() const override;
		virtual bool ForceUpdate() override;

		void HandleMessage(const std::string &message, const std::string & /*topic*/) override;
};

}

#endif

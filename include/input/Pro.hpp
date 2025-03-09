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

#ifndef __INPUT_PRO_HPP__
#define __INPUT_PRO_HPP__

#include <input/MQTT.hpp>

#include <string>

namespace configuration {
	class Json;
}

namespace input {

class Pro: public MQTT
{
	public:
		Pro(const std::string &mqtt_id, int input, const std::string &ip = ""): MQTT(mqtt_id, input, ip) {}
		virtual ~Pro() {}

		static void CheckConfig(const configuration::Json &conf);
};

}

#endif

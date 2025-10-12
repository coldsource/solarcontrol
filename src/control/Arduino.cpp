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

#include <control/Arduino.hpp>
#include <configuration/Json.hpp>
#include <mqtt/Client.hpp>
#include <excpt/Config.hpp>
#include <excpt/Context.hpp>

using namespace std;

namespace control {

void Arduino::CheckConfig(const configuration::Json &conf)
{
	OnOff::CheckConfig(conf);

	conf.Check("mqtt_id", "string");
	if(conf.GetString("mqtt_id")=="")
		throw excpt::Config("Missing MQTT ID", "mqtt_id");

	conf.Check("reverted", "bool", false);
}

void Arduino::Switch(bool new_state)
{
	excpt::Context ctx("control", "Setting switch state");

	if(reverted)
		new_state = !new_state;

	mqtt::Client::GetInstance()->Publish(topic, new_state?"RELAY ON":"RELAY OFF");
}

}



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

#include <sensor/meter/Arduino.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::meter {

Arduino::Arduino(const string &name, const string &mqtt_id):Meter(name), last_update(TS_MONOTONIC)
{
	mqtt_topics = {mqtt_id};
}

Arduino::~Arduino()
{
}

bool Arduino::HandleMQTTMessage(const string &message, const std::string & /*topic*/)
{
	try
	{
		json j = json::parse(message);

		if(j.contains("voltage"))
			voltage = (double)j["voltage"];

		if(j.contains("current") && voltage!=-1)
		{
			datetime::Timestamp now(TS_MONOTONIC);
			double current = j["current"];

			energy_consumption = energy_consumption + (voltage * current * (double)(now - last_update) / 3600);
			power = voltage * current;

			last_update = now;
		}
	}
	catch(json::exception &e)
	{
		return false;
	}

	return true;
}

}




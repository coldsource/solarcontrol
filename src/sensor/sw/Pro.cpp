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

#include <sensor/sw/Pro.hpp>
#include <logs/Logger.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::sw {

Pro::Pro(const string &ip, int outlet, const string &mqtt_id): Relay(ip, outlet, mqtt_id)
{
}

void Pro::CheckConfig(const configuration::Json &conf)
{
	Relay::CheckConfig(conf);

	conf.Check("outlet", "int");
}

}


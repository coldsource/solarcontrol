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

#ifndef __SENSOR_SW_PLUG_HPP__
#define __SENSOR_SW_PLUG_HPP__

#include <sensor/sw/Relay.hpp>

#include <string>

namespace sensor::sw {

class Plug: public Relay
{
	public:
		Plug(const std::string &ip, const std::string &mqtt_id, bool reverted): Relay(ip, 0, mqtt_id, reverted) {}
};

}

#endif


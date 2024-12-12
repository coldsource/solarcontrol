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

#ifndef __METER_PLUG_HPP__
#define __METER_PLUG_HPP__

#include <meter/Meter.hpp>
#include <mqtt/Subscriber.hpp>

#include <string>

namespace configuration {
	class Json;
}

namespace meter {

class Plug: public Meter, public mqtt::Subscriber
{
	std::string topic = "";

	public:
		Plug(const std::string &mqtt_id);
		virtual ~Plug();

		void HandleMessage(const std::string &message);
};

}

#endif



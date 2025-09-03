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

#ifndef __CONTROL_RELAY_HPP__
#define __CONTROL_RELAY_HPP__

#include <control/OnOff.hpp>

#include <string>
#include <mutex>
#include <atomic>

namespace configuration {
	class Json;
}

namespace control {

class Relay: public OnOff
{
	const std::string ip = "";
	const int outlet = 0;

	bool reverted = false;

	std::mutex lock;

	public:
		Relay(const std::string &ip, int outlet, bool reverted = false):ip(ip), outlet(outlet), reverted(reverted) {}
		virtual ~Relay() {}

		static void CheckConfig(const configuration::Json & conf);

		void Switch(bool state) override;
};

}

#endif



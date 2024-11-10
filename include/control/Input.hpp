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

#ifndef __CONTROL_INPUT_HPP__
#define __CONTROL_INPUT_HPP__

#include <control/HTTP.hpp>
#include <mqtt/Subscriber.hpp>

#include <string>
#include <mutex>

namespace control {

class Input: public HTTP, public mqtt::Subscriber
{
	std::string topic;

	bool state = false;
	int input = 0;

	mutable std::mutex lock;

	protected:
		bool get_input() const;

	public:
		Input(const std::string &mqtt_id, int input, const std::string &ip = "");
		virtual ~Input();

		bool GetState() const { return state; }
		void UpdateState();

		void HandleMessage(const std::string &message);
};

}

#endif

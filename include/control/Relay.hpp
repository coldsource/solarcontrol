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

#include <control/HTTP.hpp>
#include <control/OnOff.hpp>
#include <mqtt/Subscriber.hpp>

#include <string>
#include <mutex>
#include <atomic>

namespace configuration {
	class Json;
}

namespace control {

class Relay: public HTTP, public OnOff, public mqtt::Subscriber
{
	const int outlet = 0;
	const std::string topic = "";

	std::atomic_bool state = false;

	std::mutex lock;

	protected:
		bool get_output() const;

	public:
		Relay(const std::string &ip, int outlet, const std::string &mqtt_id);
		virtual ~Relay();

		static void CheckConfig(const configuration::Json & conf);

		void Switch(bool state) override;
		bool GetState() const override;
		void UpdateState() override;

		void HandleMessage(const std::string &message, const std::string & /*topic*/) override;
};

}

#endif



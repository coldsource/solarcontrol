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

#ifndef __SENSOR_SW_RELAY_HPP__
#define __SENSOR_SW_RELAY_HPP__

#include <sensor/sw/Switch.hpp>
#include <mqtt/Subscriber.hpp>

#include <string>
#include <mutex>
#include <atomic>

namespace configuration {
	class Json;
}

namespace sensor::sw {

class Relay: public Switch, public mqtt::Subscriber
{
	// Config
	const std::string ip;
	const int outlet = 0;
	const std::string topic;
	const bool reverted = false;

	// State
	std::atomic_bool state = false;
	std::atomic_bool manual = false;

	protected:
		bool get_output() const;

	public:
		Relay(const std::string &ip, int outlet, const std::string &mqtt_id, bool reverted = false);
		virtual ~Relay();

		static void CheckConfig(const configuration::Json & conf);

		bool GetState() const override;
		bool ForceUpdate() override;
		bool IsManual() const override { return manual; }

		void HandleMessage(const std::string &message, const std::string & /*topic*/) override;
};

}

#endif



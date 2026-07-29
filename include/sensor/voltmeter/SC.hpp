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

#ifndef __SENSOR_VOLTMETER_SC_HPP__
#define __SENSOR_VOLTMETER_SC_HPP__

#include <sensor/voltmeter/Voltmeter.hpp>
#include <mqtt/Subscriber.hpp>

#include <string>

namespace configuration {
	class Json;
}

namespace sensor::voltmeter {

/*
 * Meter for monitoring battery voltate throught Shelly Uni API
 * State Of Charge (SOC) is deduced from average voltage
 */

class SC: public Voltmeter, public mqtt::Subscriber
{
	std::string topic_ina;
	std::string topic_soc;

	// State
	charge_state_t charge_state = UNKNOWN;
	double soc = -1;
	double v = -1;

	public:
		SC(const configuration::Json &conf);
		virtual ~SC();

		static void CheckConfig(const configuration::Json &conf);

		virtual double GetVoltage() const override { return v; }
		virtual double GetSOC() const override { return soc; };
		virtual charge_state_t GetState() const override { return charge_state; }
		bool IsCharging() const override { return charge_state==CHARGING; }

		void HandleMessage(const std::string &message, const std::string &topic) override;
};

}

#endif


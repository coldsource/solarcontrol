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

#ifndef __SENSOR_VOLTMETER_UNI_HPP__
#define __SENSOR_VOLTMETER_UNI_HPP__

#include <sensor/voltmeter/Voltmeter.hpp>
#include <mqtt/Subscriber.hpp>
#include <configuration/ConfigurationObserver.hpp>
#include <datetime/Timestamp.hpp>
#include <stat/MovingAverage.hpp>

#include <string>

namespace configuration {
	class Json;
}

namespace sensor::voltmeter {

/*
 * Meter for monitoring battery voltate throught Shelly Uni API
 * State Of Charge (SOC) is deduced from average voltage
 */

class Uni: public Voltmeter, public mqtt::Subscriber, public configuration::ConfigurationObserver
{
	std::string topic;

	// State
	std::atomic<std::shared_ptr<stat::MovingAverage<double>>> voltage_avg; // Average voltage in mV
	std::atomic<datetime::Timestamp> last_voltage_update;
	std::atomic_bool charging = false;

	public:
		Uni(const configuration::Json &conf);
		virtual ~Uni();

		static void CheckConfig(const configuration::Json &conf);
		void ConfigurationChanged(const configuration::ConfigurationPart *config) override;

		double GetVoltage() const override;
		bool IsCharging() const override { return charging; }

		void HandleMessage(const std::string &message, const std::string & /*topic*/) override;
};

}

#endif

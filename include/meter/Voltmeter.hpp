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

#ifndef __METER_VOLTMETER_HPP__
#define __METER_VOLTMETER_HPP__

#include <meter/Meter.hpp>
#include <mqtt/Subscriber.hpp>
#include <configuration/ConfigurationObserver.hpp>
#include <datetime/Timestamp.hpp>

#include <string>
#include <mutex>
#include <map>
#include <memory>

namespace configuration {
	class Configuration;
}

namespace stat {
	class MovingAverage;
}

namespace meter {

class Voltmeter: public mqtt::Subscriber, public configuration::ConfigurationObserver
{
	mutable std::mutex lock;

	std::string topic = "";

	std::atomic<std::shared_ptr<stat::MovingAverage>> voltage_avg; // Average voltage in mV
	std::atomic<datetime::Timestamp> last_voltage_update;

	std::map<int, double> thresholds;

	public:
		Voltmeter(const configuration::Json &conf);
		virtual ~Voltmeter();

		static void CheckConfig(const configuration::Json &conf);
		void ConfigurationChanged(const configuration::Configuration *config);

		void SetVoltage(double v);
		double GetVoltage() const;
		double GetSOC() const;

		void HandleMessage(const std::string &message, const std::string & /*topic*/);
};

}

#endif




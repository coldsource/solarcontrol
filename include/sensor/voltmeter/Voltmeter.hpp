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

#ifndef __SENSOR_VOLTMETER_VOLTMETER_HPP__
#define __SENSOR_VOLTMETER_VOLTMETER_HPP__

#include <sensor/Sensor.hpp>
#include <configuration/ConfigurationObserver.hpp>
#include <datetime/Timestamp.hpp>
#include <stat/MovingAverage.hpp>

#include <string>
#include <mutex>
#include <map>

namespace configuration {
	class Json;
}

namespace sensor::voltmeter {

class Voltmeter: public Sensor, public configuration::ConfigurationObserver
{
	protected:
		mutable std::mutex lock;

		// Config
		unsigned long smoothing;
		double charge_delta;
		double max_voltage;
		std::map<int, double> thresholds; // Voltate thresholds used for computing SOC

		// State
		std::shared_ptr<stat::MovingAverage<double>> voltage_avg; // Average voltage in mV
		datetime::Timestamp last_voltage_update;
		datetime::Timestamp last_reload;

		bool prevent_notify() const;

	public:
		Voltmeter(const configuration::Json &conf);
		virtual ~Voltmeter() {}

		static void CheckConfig(const configuration::Json &conf);
		void ConfigurationChanged(const configuration::ConfigurationPart *config) override;

		virtual double GetVoltage() const;
		double GetSOC() const;
		virtual bool IsCharging() const = 0;

		virtual std::string GetCategory() const override { return "voltmeter"; }
};

}

#endif




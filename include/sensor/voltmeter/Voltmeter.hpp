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

#include <string>
#include <mutex>
#include <map>

namespace configuration {
	class Json;
}

namespace sensor::voltmeter {

class Voltmeter: public Sensor
{
	protected:
		mutable std::mutex lock;

		// Config
		double charge_delta;
		double max_voltage;
		std::map<int, double> thresholds; // Voltate thresholds used for computing SOC

	public:
		Voltmeter(const configuration::Json &conf);
		virtual ~Voltmeter() {}

		static void CheckConfig(const configuration::Json &conf);

		virtual double GetVoltage() const = 0;
		virtual bool IsCharging() const = 0;
		double GetSOC() const;

		virtual std::string GetCategory() const override { return "voltmeter"; }
};

}

#endif




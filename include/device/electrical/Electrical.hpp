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

#ifndef __DEVICE_ELECTRICAL_HPP__
#define __DEVICE_ELECTRICAL_HPP__

#include <configuration/ConfigurationObserver.hpp>
#include <device/Device.hpp>
#include <energy/Counter.hpp>

#include <string>
#include <memory>
#include <atomic>

namespace control {
	class OnOff;
}

namespace device {

class Electrical: public Device, public configuration::ConfigurationObserver
{
	protected:
		// State
		std::atomic<double> power = 0;

		// Global config
		bool debug = false;
		double debug_grid = 0;
		double debug_pv = 0;
		double debug_battery = 0;
		double debug_hws = 0;

		energy::Counter consumption;
		energy::Counter offload;

		virtual void reload(const configuration::Json &config) override;

	public:
		Electrical(int id);
		Electrical(int id, const std::string &consumption_type, const std::string consumption_excess_type);
		virtual ~Electrical();

		void ConfigurationChanged(const configuration::ConfigurationPart *config) override;

		static void CheckConfig(const configuration::Json &conf);

		double GetPower() const;
		bool IsMetered() const;

		energy::Amount GetEnergyConsumption() const;
		energy::Amount GetEnergyOffload() const;
		energy::Amount GetEnergyExcess() const;

		const std::map<datetime::Date, energy::Amount> &GetConsumptionHistory() const;
		const std::map<datetime::Date, energy::Amount> &GetOffloadHistory() const;

		virtual nlohmann::json ToJson() const override;

		virtual void SensorChanged(const sensor::Sensor *sensor) override;
};

}

#endif

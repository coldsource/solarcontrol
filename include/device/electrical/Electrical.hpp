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
	private:
		// State
		std::atomic<double> power = 0;
		std::atomic<double> voltage = 0;
		std::atomic<double> freq = 0;

		energy::Counter consumption;
		energy::Counter offload;

		// Global config
		struct st_debug
		{
			bool enabled = false;
			double grid = 0;
			double pv = 0;
			double hws = 0;
			double battery = 0;
			int battery_soc = 0;
		};

		std::atomic<std::shared_ptr<st_debug>> debug;
		void energy_debug();

	protected:
		// Device configuration
		class Configuration: public Device::Configuration
		{
			public:
				bool is_metered;

				Configuration(const configuration::Json &config);

				virtual void Prepare(Device *device) override;
		};

		inline std::shared_ptr<const Configuration> get_config() const { return dynamic_pointer_cast<const Electrical::Configuration>(config.load()); }
		inline std::shared_ptr<st_debug> get_debug() const { return debug.load(); }

		virtual void sensor_changed(const sensor::Sensor *sensor) override;

	public:
		Electrical(int id);
		Electrical(int id, const std::string &consumption_type, const std::string consumption_excess_type);
		virtual ~Electrical();

		void ConfigurationChanged(const configuration::ConfigurationPart *config) override;

		static void CheckConfig(const configuration::Json &conf);

		virtual bool IsMetered() const final { return get_config()->is_metered; }
		virtual double GetPower() const;
		virtual double GetVoltage() const;
		virtual double GetFrequency() const;

		virtual void SyncHistory() override;

		energy::Amount GetEnergyConsumption() const;
		energy::Amount GetEnergyOffload() const;
		energy::Amount GetEnergyExcess() const;
		double GetTotalConsumptionForLast(int ndays) const;

		const std::map<datetime::Date, energy::Amount> GetConsumptionHistory() const;
		const std::map<datetime::Date, energy::Amount> GetConsumptionExcessHistory() const;
		const std::map<datetime::Date, energy::Amount> GetOffloadHistory() const;

		virtual nlohmann::json ToJson() const override;

};

}

#endif

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

#ifndef __THREAD_STATS_HPP__
#define __THREAD_STATS_HPP__

#include <thread/WaiterThread.hpp>
#include <stat/MovingAverage.hpp>
#include <stat/CumulativeHigherFrequency.hpp>
#include <configuration/ConfigurationObserver.hpp>
#include <nlohmann/json.hpp>

#include <mutex>
#include <memory>

namespace energy {
	class GlobalMeter;
}

namespace device {
	class DeviceOnOff;
}

namespace thread {

class Stats: public WaiterThread, public configuration::ConfigurationObserver
{
	enum en_device_speed {SLOW, MEDIUM, FAST};
	enum en_weather_type {VARIABLE, CONTANT};

	static Stats *instance;

	mutable std::mutex lock;

	// Config
	int hysteresis_export = 0;
	unsigned long slow_time;
	unsigned long medium_time;
	unsigned long fast_time;


	protected:
		// Controllable power is the sum of power that can be manager by Solar Control
		// It includes available (exported) + Power manager by controlled OnOff devices (Forced + Offload)
		energy::GlobalMeter *global_meter = 0;
		std::unique_ptr<stat::MovingAverage<double>> controllable_power_avg; // Average controllable power, used to switch devices off
		std::unique_ptr<stat::CumulativeHigherFrequency<double>> controllable_power_frequency_fast; // Statistic distribution of controlled power for fast devices
		std::unique_ptr<stat::CumulativeHigherFrequency<double>> controllable_power_frequency_medium; // Statistic distribution of controlled power for medium devices
		std::unique_ptr<stat::CumulativeHigherFrequency<double>> controllable_power_frequency_slow; // Statistic distribution of controlled power for slow devices
		std::unique_ptr<stat::CumulativeHigherFrequency<double>> production_power_frequency; // Statistic distribution of production
		double controllable_power = 0;
		double controlled_power = 0;

		double get_controlled_power() const;
		double get_device_prediction(const std::shared_ptr<device::DeviceOnOff> &device, double active_power = 0) const;
		en_weather_type get_weather_type() const;
		en_device_speed get_device_type(const std::shared_ptr<device::DeviceOnOff> &device) const;
		nlohmann::json frequency_to_json(const std::unique_ptr<stat::CumulativeHigherFrequency<double>> &frequency) const;
		nlohmann::json devices_predictions_to_json() const;

		void main(void) override;

	public:
		Stats();
		virtual ~Stats();

		static Stats *GetInstance() { return instance; }

		void Start() { start(); }

		double GetControllablePowerAvg() const;
		double GetDevicePrediction(const std::shared_ptr<device::DeviceOnOff> &device, double active_power) const;

		nlohmann::json ToJson() const;

		void ConfigurationChanged(const configuration::ConfigurationPart *config) override;
};

}

#endif



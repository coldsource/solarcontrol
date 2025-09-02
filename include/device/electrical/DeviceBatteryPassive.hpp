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

#ifndef __DEVICE_DEVICEBATTERYPASSIVE_HPP__
#define __DEVICE_DEVICEBATTERYPASSIVE_HPP__

#include <device/electrical/DevicePassive.hpp>

#include <memory>

namespace configuration {
	class Json;
}

namespace device {

class DeviceBatteryPassive: public DevicePassive
{
	// State
	double voltage = -1, soc = -1;

	protected:
		virtual void reload(const configuration::Json &config) override;
		virtual void state_restore(const  configuration::Json &last_state) override;
		virtual configuration::Json state_backup() override;

	public:
		DeviceBatteryPassive(int id);
		virtual ~DeviceBatteryPassive();

		std::string GetType() const override { return "battery-passive"; }

		const std::map<datetime::Date, energy::Amount> &GetProductionHistory() const { return consumption.GetConsumptionHistory(); }

		static void CheckConfig(const configuration::Json &conf);

		double GetVoltage() const { return voltage; }
		double GetSOC() const { return soc; }
		virtual nlohmann::json ToJson() const override;

		virtual void SensorChanged(const sensor::Sensor *sensor) override;
};

}

#endif



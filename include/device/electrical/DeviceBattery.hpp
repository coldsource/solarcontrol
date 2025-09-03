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

#ifndef __DEVICE_DEVICEBATTERY_HPP__
#define __DEVICE_DEVICEBATTERY_HPP__

#include <device/electrical/DeviceOnOff.hpp>
#include <datetime/Timestamp.hpp>

#include <memory>

namespace configuration {
	class Json;
}

namespace device {

class DeviceBattery: public DeviceOnOff
{
	// Config
	unsigned int battery_low, battery_high;
	unsigned long min_grid_time;

	// State
	double voltage = -1, soc = -1;
	datetime::Timestamp last_grid_switch;

	protected:
		virtual void reload(const configuration::Json &config) override;
		virtual void state_restore(const  configuration::Json &last_state) override;
		virtual configuration::Json state_backup() override;

	public:
		DeviceBattery(int id);
		virtual ~DeviceBattery();

		std::string GetType() const override { return "battery"; }

		const std::map<datetime::Date, energy::Amount> &GetProductionHistory() const { return consumption.GetConsumptionHistory(); }

		static void CheckConfig(const configuration::Json &conf);

		virtual en_wanted_state GetWantedState() const override;

		virtual void SetState(bool new_state) override;

		double GetVoltage() const { return voltage; }
		double GetSOC() const { return soc; }
		virtual nlohmann::json ToJson() const override;

		virtual void SensorChanged(const sensor::Sensor *sensor) override;

		static void CreateInDB();
};

}

#endif


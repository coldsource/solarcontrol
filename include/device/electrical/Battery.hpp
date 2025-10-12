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

#ifndef __DEVICE_BATTERY_HPP__
#define __DEVICE_BATTERY_HPP__

#include <device/electrical/OnOff.hpp>
#include <datetime/Timestamp.hpp>

#include <memory>

namespace configuration {
	class Json;
}

namespace device {

class Battery: public OnOff
{
	public:
		enum en_battery_policy {GRID, BATTERY, OFFLOAD};
		enum en_battery_state {DISCHARGING, CHARGING, FLOAT, BACKUP};
		enum en_grid_state {UNKNOWN, ONLINE, OFFLINE};

	private:
		// Config
		bool enabled;
		unsigned int battery_low, battery_high;
		unsigned long min_grid_time;
		unsigned long battery_cooldown;
		en_battery_policy policy;

		// State
		double voltage = -1, soc = -1;
		datetime::Timestamp last_grid_switch;
		en_battery_state soc_state = FLOAT;
		en_grid_state grid_state = UNKNOWN;

	protected:
		virtual void reload(const configuration::Json &config) override;
		virtual void state_restore(const  configuration::Json &last_state) override;
		virtual configuration::Json state_backup() override;

		static en_battery_policy string_to_policy(const std::string &str);
		static std::string policy_to_string(en_battery_policy policy);
		static en_battery_state string_to_state(const std::string &str);
		static std::string state_to_string(en_battery_state state);
		en_grid_state string_to_grid_state(const std::string &str);
		static std::string grid_state_to_string(en_grid_state state);

	public:
		Battery(int id);
		virtual ~Battery();

		void ConfigurationChanged(const configuration::ConfigurationPart * config) override;

		bool IsEnabled() { return enabled; }

		std::string GetType() const override { return "battery"; }
		virtual unsigned long GetMinOn() override { return battery_cooldown; }

		const std::map<datetime::Date, energy::Amount> &GetProductionHistory() const { return consumption.GetConsumptionHistory(); }

		static void CheckConfig(const configuration::Json &conf);

		virtual en_wanted_state GetWantedState() const override;

		double GetVoltage() const { return voltage; }
		double GetSOC() const { return soc; }
		virtual nlohmann::json ToJson() const override;

		virtual void SensorChanged(const sensor::Sensor *sensor) override;

		virtual double GetExpectedConsumption() const override;

		static void CreateInDB();
};

}

#endif

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

#ifndef __DEVICE_ONOFF_HPP__
#define __DEVICE_ONOFF_HPP__

#include <device/electrical/Electrical.hpp>
#include <datetime/TimespanHistory.hpp>
#include <datetime/Timestamp.hpp>

#include <string>
#include <memory>
#include <atomic>

namespace device {

enum en_wanted_state
{
	ON,
	OFF,
	OFFLOAD,
	UNCHANGED
};

class OnOff: public Electrical
{
	private:
		// State
		std::atomic_bool manual = false;
		std::atomic_int manual_timeout = 0;
		std::atomic_bool state = false;

		datetime::Timestamp last_on;
		datetime::Timestamp last_off;
		datetime::Timestamp last_manual;

		datetime::TimespanHistory on_history;

	protected:
		// Device config
		class Configuration: public Electrical::Configuration
		{
			public:
				std::shared_ptr<control::OnOff> ctrl = nullptr;

				int prio;
				double expected_consumption = 0;

				// Battery configuration
				bool battery_enabled = false;
				int battery_controller_id = 0;

				Configuration(const configuration::Json &config);
		};

		inline std::shared_ptr<const Configuration> get_config() const { return dynamic_pointer_cast<const OnOff::Configuration>(config.load()); }

		void clock(bool new_state);
		virtual void state_restore(const  configuration::Json &last_state) override;
		virtual void sensor_changed(const sensor::Sensor *sensor) override;
		virtual configuration::Json state_backup() override;

	public:
		OnOff(int id): Electrical(id), on_history(id) {}
		OnOff(int id, const std::string &consumption_type, const std::string consumption_excess_type): Electrical(id, consumption_type, consumption_excess_type), on_history(id) {}
		virtual ~OnOff() {}

		static void CheckConfig(const configuration::Json &conf);

		en_category GetCategory() const override { return ONOFF; }

		int GetPrio() const { return get_config()->prio; }
		virtual unsigned long GetMinOn() { return 0; }

		virtual en_wanted_state GetWantedState() const = 0;
		virtual bool GetState() const { return state; }
		virtual void SetState(bool new_state);
		virtual void SetManualState(bool new_state, int timeout = 0);
		virtual void SetAutoState();
		virtual bool IsManual() const { return manual; }
		virtual datetime::Timestamp GetLastManual() const;
		virtual bool IsOnBattery() const;

		virtual double GetExpectedConsumption() const;

		datetime::Timestamp GetLastOn() const;
		datetime::Timestamp GetLastOff() const;
		unsigned long GetOnHistoryTotalForLast(unsigned long nseconds) const;

		virtual nlohmann::json ToJson() const override;

		virtual bool Depends(int device_id) const override;
		virtual void SpecificActions() override;

		static bool CompareTo(std::shared_ptr<OnOff> a, std::shared_ptr<OnOff> b) // OnOff devices are sorted by prio
		{
			return a->GetPrio() < b->GetPrio();
		}
};

}

#endif


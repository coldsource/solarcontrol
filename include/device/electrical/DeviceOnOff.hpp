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

#ifndef __DEVICE_DEVICEONOFF_HPP__
#define __DEVICE_DEVICEONOFF_HPP__

#include <device/electrical/DeviceElectrical.hpp>
#include <datetime/TimespanHistory.hpp>
#include <datetime/Timestamp.hpp>

#include <string>
#include <memory>

namespace device {

enum en_wanted_state
{
	ON,
	OFF,
	OFFLOAD,
	UNCHANGED
};

class DeviceOnOff: public DeviceElectrical
{
	protected:
		std::shared_ptr<control::OnOff> ctrl;

		datetime::Timestamp last_on;
		datetime::Timestamp last_off;

		int prio;
		double expected_consumption;

		datetime::TimespanHistory on_history;

		void clock(bool new_state);
		virtual void reload(const configuration::Json &config) override;

	public:
		DeviceOnOff(int id);
		virtual ~DeviceOnOff();

		static void CheckConfig(const configuration::Json &conf);

		en_category GetCategory() const override { return ONOFF; }

		int GetPrio() const { return prio; }
		virtual unsigned long GetMinOn() { return 0; }

		virtual en_wanted_state GetWantedState() const = 0;
		virtual bool GetState() const { return state; }
		virtual void SetState(bool new_state);
		virtual void SetManualState(bool new_state);
		virtual void SetAutoState();
		virtual bool IsManual() const { return manual; }

		double GetExpectedConsumption() const;

		virtual void SensorChanged(const sensor::Sensor *sensor) override;

		virtual nlohmann::json ToJson() const override;

		struct PrioComparator {
			bool operator()(std::shared_ptr<DeviceOnOff> a, std::shared_ptr<DeviceOnOff> b) const { return a->GetPrio() < b->GetPrio(); }
		};
};

}

#endif


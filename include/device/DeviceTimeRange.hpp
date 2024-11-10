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

#ifndef __DEVICE_DEVICETIMERANGE_HPP__
#define __DEVICE_DEVICETIMERANGE_HPP__

#include <device/DeviceOnOff.hpp>
#include <datetime/TimeRanges.hpp>
#include <datetime/TimespanHistory.hpp>
#include <datetime/Timestamp.hpp>
#include <control/OnOff.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace energy {
	class GlobalMeter;
}

namespace configuration {
	class Json;
}

namespace device {

class DeviceTimeRange: public DeviceOnOff
{
	protected:
		const energy::GlobalMeter *global_meter;
		control::OnOff *ctrl;

		bool manual = false;

		bool need_update = true; // Force state update on reload

		int min_on = 0;
		int max_on = 0;
		int min_off = 0;
		datetime::Timestamp last_on;
		datetime::Timestamp last_off;

		datetime::TimeRanges force;

		datetime::TimeRanges offload;
		double expected_consumption;

		datetime::TimeRanges remainder;
		int min_on_time;
		int min_on_for_last;

		datetime::TimespanHistory on_history;

		virtual bool state_on_condition() const { return true; }

	public:
		DeviceTimeRange(unsigned int id, const std::string &name, const configuration::Json &config);
		virtual ~DeviceTimeRange();

		std::string GetType() const { return "timerange"; }

		virtual bool IsForced() const;
		virtual bool WantOffload() const;
		virtual bool WantRemainder() const;

		double GetExpectedConsumption() const { return expected_consumption; }

		bool WantedState() const;
		bool GetState() const;
		void SetState(bool new_state);
		void SetManualState(bool new_state);
		void SetAutoState();
		bool IsManual() { return manual; }
		void UpdateState();
		bool NeedStateUpdate() { return need_update; }
};

}

#endif


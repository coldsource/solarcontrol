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
#include <control/OnOff.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace energy {
	class GlobalMeter;
}

namespace device {

class DeviceTimeRange: public DeviceOnOff
{
	const energy::GlobalMeter *global_meter;
	control::OnOff *ctrl;

	bool manual = false;

	datetime::TimeRanges force;

	datetime::TimeRanges offload;
	double expected_consumption;

	datetime::TimeRanges remainder;
	datetime::TimespanHistory on_history;
	int min_on_time;
	int min_on_for_last;

	public:
		DeviceTimeRange(unsigned int id, const std::string &name, const nlohmann::json &config);
		virtual ~DeviceTimeRange();

		std::string GetType() const { return "timerange-plug"; }

		bool IsForced() const;
		bool WantOffload() const;
		bool WantRemainder() const;

		double GetExpectedConsumption() const { return expected_consumption; }

		bool WantedState() const;
		bool GetState() const;
		void SetState(bool new_state);
		void SetManualState(bool new_state);
		void SetAutoState();
		bool IsManual() { return manual; }
		void UpdateState();
};

}

#endif


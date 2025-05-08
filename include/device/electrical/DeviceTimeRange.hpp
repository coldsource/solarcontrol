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

#include <device/electrical/DeviceOnOff.hpp>
#include <datetime/TimeRanges.hpp>

#include <string>
#include <vector>

namespace configuration {
	class Json;
}

namespace device {

class DeviceTimeRange: public DeviceOnOff
{
	protected:
		unsigned long min_on = 0;
		unsigned long max_on = 0;
		unsigned long min_off = 0;

		datetime::TimeRanges force;

		datetime::TimeRanges offload;

		datetime::TimeRanges remainder;
		unsigned long min_on_time;
		unsigned long min_on_for_last;

		virtual en_wanted_state get_wanted_state(configuration::Json *data_ptr = 0) const;

	public:
		DeviceTimeRange(unsigned int id, const std::string &name, const configuration::Json &config);
		virtual ~DeviceTimeRange();

		static void CheckConfig(const configuration::Json &conf);

		std::string GetType() const { return "timerange"; }

		virtual bool IsForced(configuration::Json *data_ptr = 0) const;
		virtual bool WantOffload(configuration::Json *data_ptr = 0) const;
		virtual bool WantRemainder(configuration::Json *data_ptr = 0) const;

		virtual en_wanted_state GetWantedState() const;
};

}

#endif


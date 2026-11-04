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

#ifndef __DEVICE_TIMERANGE_HPP__
#define __DEVICE_TIMERANGE_HPP__

#include <device/electrical/OnOff.hpp>
#include <datetime/TimeRanges.hpp>

#include <string>
#include <vector>

namespace configuration {
	class Json;
}

namespace device {

class TimeRange: public OnOff
{
	protected:
		class Configuration: public OnOff::Configuration
		{
			public:
				unsigned long min_on = 0;
				unsigned long max_on = 0;
				unsigned long min_off = 0;

				datetime::TimeRanges force;
				datetime::TimeRanges offload;
				datetime::TimeRanges remainder;
				unsigned long min_on_time;
				unsigned long min_on_for_last;

				Configuration(const configuration::Json &config);
		};

		virtual en_wanted_state get_wanted_state(configuration::Json *data_ptr = 0) const;

		virtual std::shared_ptr<Device::Configuration> build_config(const configuration::Json &j_config) const override;
		inline std::shared_ptr<const Configuration> get_config() const { return dynamic_pointer_cast<const TimeRange::Configuration>(config.load()); }

	public:
		TimeRange(int id);
		virtual ~TimeRange();

		static void CheckConfig(const configuration::Json &conf);

		std::string GetType() const override { return "timerange"; }

		virtual unsigned long GetMinOn() override;

		virtual bool IsForced(configuration::Json *data_ptr = 0) const;
		virtual bool WantOffload(configuration::Json *data_ptr = 0) const;
		virtual bool WantRemainder(configuration::Json *data_ptr = 0) const;

		virtual en_wanted_state GetWantedState() const override;
		virtual configuration::Json GetCurrentTimerangeData() const;
};

}

#endif


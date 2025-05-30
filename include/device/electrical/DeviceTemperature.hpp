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

#ifndef __DEVICE_DEVICETEMPERATURE_HPP__
#define __DEVICE_DEVICETEMPERATURE_HPP__

#include <device/electrical/DeviceTimeRange.hpp>
#include <configuration/ConfigurationObserver.hpp>

namespace configuration {
	class Json;
}

namespace device {

class DeviceTemperature: public DeviceTimeRange, public configuration::ConfigurationObserver
{
	protected:
		int ht_device_id;

		double absence_temperature;
		bool absence;

		static void check_timeranges(const configuration::Json &conf, const std::string &name);

		virtual en_wanted_state get_wanted_state(configuration::Json *data_ptr = 0) const;
		virtual bool temp_check_force(double current_temp, double timerange_temp) const = 0;
		virtual bool temp_check_offload(double current_temp, double timerange_temp) const = 0;

	public:
		DeviceTemperature(unsigned int id, const std::string &name, const configuration::Json &config);
		virtual ~DeviceTemperature();

		void ConfigurationChanged(const configuration::Configuration * config);

		static void CheckConfig(const configuration::Json &conf);

		bool Depends(int device_id) const;
};

}

#endif



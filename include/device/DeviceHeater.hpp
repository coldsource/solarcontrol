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

#ifndef __DEVICE_DEVICEHEATER_HPP__
#define __DEVICE_DEVICEHEATER_HPP__

#include <device/DeviceTimeRange.hpp>

namespace configuration {
	class Json;
}

namespace device {

class DeviceHeater: public DeviceTimeRange
{
	protected:
		int ht_device_id;
		double force_max_temperature;
		double offload_max_temperature;

	public:
		DeviceHeater(unsigned int id, const std::string &name, const configuration::Json &config);
		virtual ~DeviceHeater() {}

		static void CheckConfig(const configuration::Json &conf);

		std::string GetType() const { return "heater"; }

		en_wanted_state GetWantedState() const;

		bool Depends(int device_id) const;
};

}

#endif



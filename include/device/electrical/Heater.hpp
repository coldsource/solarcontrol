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

#ifndef __DEVICE_HEATER_HPP__
#define __DEVICE_HEATER_HPP__

#include <device/electrical/Temperature.hpp>

namespace device {

class Heater: public Temperature
{
	protected:
		virtual bool temp_check_force(double current_temp, double timerange_temp) const override;
		virtual bool temp_check_offload(double current_temp, double timerange_temp) const override;

	public:
		Heater(int id): Temperature(id) {}
		virtual ~Heater() {}

		std::string GetType() const override { return "heater"; }
};

}

#endif




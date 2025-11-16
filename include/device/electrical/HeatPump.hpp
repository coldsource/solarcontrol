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

#ifndef __DEVICE_HEATPUMP_HPP__
#define __DEVICE_HEATPUMP_HPP__

#include <device/electrical/Heater.hpp>
#include <datetime/Timer.hpp>

namespace coniguration {
	class Json;
}

namespace device {

class HeatPump: public Temperature
{
	protected:
		// State
		datetime::Timer update_interval;

		// Config
		double temperature_offset_slow;
		double temperature_offset_fast;
		double temperature_eco;
		double temperature_comfort;

		virtual void reload(const configuration::Json &config) override;

		virtual bool temp_check_force(double current_temp, double timerange_temp) const override;
		virtual bool temp_check_offload(double current_temp, double timerange_temp) const override;

	public:
		HeatPump(int id);
		virtual ~HeatPump() {}

		static void CheckConfig(const configuration::Json &conf);

		std::string GetType() const override { return "heatpump"; }

		virtual void SpecificActions() override;
};

}

#endif





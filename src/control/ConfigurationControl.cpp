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

#include <control/ConfigurationControl.hpp>

#include <stdexcept>

using namespace std;

namespace configuration
{

ConfigurationControl *ConfigurationControl::instance_control;

static auto init = Configuration::GetInstance()->RegisterConfig(new ConfigurationControl());

ConfigurationControl::ConfigurationControl(void)
{
	// Load default configuration
	entries["control.hysteresis.smoothing"] = "3m";
	entries["control.hysteresis.export"] = "50W";
	entries["control.hysteresis.import"] = "30W";
	entries["control.cooldown"] = "10s";
	entries["control.state.update_interval"] = "1m";
	entries["control.presence"] = "yes";
	entries["control.absence.temperature"] = "12";

	instance_control = this;
}

ConfigurationControl::~ConfigurationControl(void)
{
}

void ConfigurationControl::Check(void)
{
	check_time_entry("control.hysteresis.smoothing");
	check_power_entry("control.hysteresis.export");
	check_power_entry("control.hysteresis.import");
	check_time_entry("control.cooldown");
	check_time_entry("control.state.update_interval");
	check_bool_entry("control.presence");
	check_double_entry("control.absence.temperature");
}

}



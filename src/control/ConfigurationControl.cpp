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
#include <configuration/Configuration.hpp>

#include <stdexcept>

using namespace std;

namespace configuration
{

static auto init = Configuration::GetInstance()->RegisterConfig(make_shared<ConfigurationControl>());

ConfigurationControl::ConfigurationControl(void)
{
	// Load default configuration
	entries["control.hysteresis.smoothing"] = "3m";
	entries["control.hysteresis.export"] = "50W";
	entries["control.hysteresis.import"] = "30W";
	entries["control.hysteresis.precision"] = "95%";
	entries["control.hysteresis.fast"] = "1m";
	entries["control.hysteresis.medium"] = "5m";
	entries["control.hysteresis.slow"] = "10m";
	entries["control.cooldown"] = "10s";
	entries["control.state.update_interval"] = "5m";
	entries["control.absence.enabled"] = "no";
	entries["control.absence.temperature"] = "12";
	entries["control.priority"] = "hws";
	entries["control.offload.max"] = "3kW";
	entries["control.battery.cooldown"] = "20m";
}

ConfigurationControl::~ConfigurationControl(void)
{
}

void ConfigurationControl::Check(void) const
{
	check_time_entry("control.hysteresis.smoothing");
	check_power_entry("control.hysteresis.export");
	check_power_entry("control.hysteresis.import");
	check_percent_entry("control.hysteresis.precision");
	check_time_entry("control.hysteresis.fast");
	check_time_entry("control.hysteresis.medium");
	check_time_entry("control.hysteresis.slow");
	check_time_entry("control.cooldown");
	check_time_entry("control.state.update_interval");
	check_bool_entry("control.absence.enabled");
	check_double_entry("control.absence.temperature");
	check_power_entry("control.offload.max");
	check_time_entry("control.battery.cooldown");

	string priority = Get("control.priority");
	if(priority!="hws" && priority!="offload")
		throw runtime_error("control.priority must be « hws » or « offload »");
}

}



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

#include <energy/ConfigurationEnergy.hpp>

#include <stdexcept>

using namespace std;

namespace configuration
{

ConfigurationEnergy *ConfigurationEnergy::instance_energy;

static auto init = Configuration::GetInstance()->RegisterConfig(new ConfigurationEnergy());

ConfigurationEnergy::ConfigurationEnergy(void)
{
	// Load default configuration
	entries["energy.hws.min"] = "3kWh";
	entries["energy.debug.enabled"] = "no";
	entries["energy.debug.grid"] = "0";
	entries["energy.debug.pv"] = "0";
	entries["energy.debug.battery"] = "0";
	entries["energy.debug.hws"] = "0";
	entries["energy.battery.smoothing"] = "5m";


	instance_energy = this;
}

ConfigurationEnergy::~ConfigurationEnergy(void)
{
}

void ConfigurationEnergy::Check(void)
{
	check_energy_entry("energy.hws.min");
	check_bool_entry("energy.debug.enabled");
	check_power_entry("energy.debug.grid", true);
	check_power_entry("energy.debug.pv", true);
	check_power_entry("energy.debug.battery", true);
	check_power_entry("energy.debug.hws", true);
	check_time_entry("energy.battery.smoothing");
}

}


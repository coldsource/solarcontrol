/*
 * This file is part of evQueue
 *
 * evQueue is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * evQueue is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with evQueue. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Thibault Kummer <bob@coldsource.net>
 */

#include <configuration/ConfigurationSolarControl.hpp>

#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdexcept>

using namespace std;

namespace configuration
{

static auto init = Configuration::GetInstance()->RegisterConfig(new ConfigurationSolarControl());

ConfigurationSolarControl::ConfigurationSolarControl(void)
{
	// Load default configuration
	entries["control.cooldown.on"] = "10";
	entries["mqtt.host"] = "127.0.0.1";
	entries["mqtt.port"] = "1883";
	entries["sql.host"] = "127.0.0.1";
	entries["sql.user"] = "";
	entries["sql.password"] = "";
	entries["sql.database"] = "sol";
}

ConfigurationSolarControl::~ConfigurationSolarControl(void)
{
}

void ConfigurationSolarControl::Check(void)
{
	check_int_entry("control.cooldown.on");

	if(GetInt("control.cooldown.on")<0)
		throw invalid_argument("control.cooldown.on: must be greater than 0");
}

}

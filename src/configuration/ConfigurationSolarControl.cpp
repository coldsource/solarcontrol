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
	entries["control.hysteresis.smoothing"] = "3";
	entries["control.hysteresis.export"] = "50";
	entries["control.hysteresis.import"] = "30";
	entries["control.cooldown"] = "10";
	entries["control.state.update_interval"] = "60";
	entries["core.history.maxdays"] = "30";
	entries["energy.hws.min"] = "3000";
	entries["energy.mqtt.id"] = "";
	entries["offpeak.mqtt.id"] = "";
	entries["offpeak.input"] = "0";
	entries["offpeak.ip"] = "";
	entries["mqtt.id"] = "solarcontrol";
	entries["mqtt.host"] = "127.0.0.1";
	entries["mqtt.port"] = "1883";
	entries["sql.host"] = "127.0.0.1";
	entries["sql.user"] = "";
	entries["sql.password"] = "";
	entries["sql.database"] = "sol";
	entries["display.lcd.path"] = "/dev/i2c-1";
	entries["display.lcd.address"] = "0x27";
	entries["display.lcd.linesize"] = "20";
	entries["display.lcd.enable"] = "no";
}

ConfigurationSolarControl::~ConfigurationSolarControl(void)
{
}

void ConfigurationSolarControl::Check(void)
{
	check_int_entry("control.hysteresis.export");
	check_int_entry("control.hysteresis.import");
	check_int_entry("control.cooldown");
	check_int_entry("core.history.maxdays");
	check_int_entry("offpeak.input");

	if(GetInt("control.cooldown")<0)
		throw invalid_argument("control.cooldown : must be greater than 0");

	if(GetInt("core.history.maxdays")<0)
		throw invalid_argument("core.history.maxdays : must be greater than 0");

	if(Get("energy.mqtt.id")=="")
		throw invalid_argument("energy.mqtt.id : can't be empty");
}

}

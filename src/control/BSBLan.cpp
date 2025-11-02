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

#include <control/BSBLan.hpp>
#include <configuration/Json.hpp>
#include <excpt/Config.hpp>
#include <excpt/BSBLan.hpp>
#include <excpt/Context.hpp>

#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace control {

void BSBLan::CheckConfig(const configuration::Json &conf)
{
	OnOff::CheckConfig(conf);

	conf.Check("ip", "string");
	if(conf.GetString("ip")=="")
		throw excpt::Config("Missing IP", "ip");
}

void BSBLan::set(int param, int value) const
{
	json params;
	params["parameter"] = param;
	params["value"] = value;
	params["type"] = 1; // Set
	send(params);

}

void BSBLan::set(int param, double value) const
{
	json params;
	params["parameter"] = param;
	params["value"] = value;
	params["type"] = 1; // Set
	send(params);
}

void BSBLan::send(const nlohmann::json &params) const
{
	excpt::Context ctx("control", "Communicating with heat pump");

	string output = http.Post("/JS", params.dump());
	json j_output = json::parse(output);
	for(auto j_item : j_output)
	{
		if(!j_item.contains("status") || j_item["status"]!=1)
			throw excpt::BSBLan("Communication failed with BSB Lan, received « " + output + " »");
	}
}

void BSBLan::Switch(bool new_state)
{
	// Mode 3 is confort
	// Mode 2 is eco
	set(700, new_state?3:2);
}

void BSBLan::SetAmbientTemperature(double temp) const
{
	set(10000, temp);
}

void BSBLan::SetEcoSetPoint(double temp) const
{
	set(712, temp);
}

void BSBLan::SetComfortSetPoint(double temp) const
{
	set(710, temp);
}

}




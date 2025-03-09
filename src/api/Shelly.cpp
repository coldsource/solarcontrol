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

#include <api/Shelly.hpp>
#include <configuration/Json.hpp>
#include <shelly/Autodetect.hpp>
#include <shelly/MQTT.hpp>
#include <shelly/Sys.hpp>
#include <nlohmann/json.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;

namespace api
{

json Shelly::HandleMessage(const string &cmd, const configuration::Json &j_params)
{
	json j_res;

	if(cmd=="autodetect")
	{
		shelly::Autodetect ad;
		return ad.GetDevices();
	}
	else if(cmd=="mqttget")
	{
		string ip = j_params.GetString("ip");

		shelly::MQTT mqtt(ip);
		return mqtt.Get();
	}
	else if(cmd=="mqttset")
	{
		string ip = j_params.GetString("ip");
		string server = j_params.GetString("server");
		string topic = j_params.GetString("topic");

		shelly::MQTT mqtt(ip);
		return mqtt.Set(server, topic);
	}
	else if(cmd=="sysset")
	{
		string ip = j_params.GetString("ip");
		string name = j_params.GetString("name");

		shelly::Sys sys(ip);
		return sys.Set(name);
	}
	else if(cmd=="reboot")
	{
		string ip = j_params.GetString("ip");

		shelly::Sys sys(ip);
		return sys.Reboot();
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « shelly »");
}

}



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

#include <sensor/weather/Wind.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::weather
{

Wind::Wind(const string &name, const string &mqtt_id):
Weather(name)
{
	mqtt_topics = {mqtt_id + "/events/rpc"};
}

Wind::~Wind()
{
}

void Wind::CheckConfig(const configuration::Json &conf)
{
	conf.Check("mqtt_id", "string");
	if(conf.GetString("mqtt_id")=="")
		throw excpt::Config("MQTT ID is mandatory", "mqtt_id");
}

double Wind::GetWind() const
{
	return wind;
}

void Wind::SetWind(double w)
{
	wind = w;
}

bool Wind::HandleMQTTMessage(const string &message, const std::string & /*topic*/)
{
	try
	{
		json j = json::parse(message);
		wind = j["params"]["input:2"]["xfreq"];
	}
	catch(json::exception &e)
	{
		return false;
	}

	return true;
}

}


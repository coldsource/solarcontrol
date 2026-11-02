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

#include <sensor/input/Grid.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Config.hpp>
#include <excpt/Context.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::input {

Grid::Grid(const string &name, const string &mqtt_id):
Input(name)
{
	state = true;

	mqtt_topics = {mqtt_id + "/grid"};
}

Grid::~Grid()
{
}

void Grid::CheckConfig(const configuration::Json &conf)
{
	Input::CheckConfig(conf);

	conf.Check("mqtt_id", "string");
	if(conf.GetString("mqtt_id")=="")
		throw excpt::Config("MQTT ID is required", "mqtt_id");
}


bool Grid::GetState() const
{
	return state;
}

bool Grid::HandleMQTTMessage(const string &message, const std::string & /*topic*/)
{
	try
	{
		json j = json::parse(message);
		if(j.contains("event") && j["event"]=="grid")
		{
			state =(bool)(j["data"]["status"]);
		}
	}
	catch(json::exception &e)
	{
		return false;
	}

	return true;
}

}


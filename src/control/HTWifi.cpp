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

#include <control/HTWifi.hpp>
#include <mqtt/Client.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace control
{

HTWifi::HTWifi(const string &mqtt_id)
{
	auto mqtt = mqtt::Client::GetInstance();
	mqtt->Subscribe(mqtt_id + "/events/rpc", this);
}

double HTWifi::GetTemperature() const
{
	unique_lock<mutex> llock(lock);

	return temperature;
}

double HTWifi::GetHumidity() const
{
	unique_lock<mutex> llock(lock);

	return humidity;
}

void HTWifi::HandleMessage(const string &message)
{
	unique_lock<mutex> llock(lock);

	try
	{
		json j = json::parse(message);
		humidity = j["params"]["humidity:0"]["rh"];
		temperature = j["params"]["temperature:0"]["tC"];
	}
	catch(json::exception &e)
	{
		return;
	}
}

}

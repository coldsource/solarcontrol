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

#include <control/HTBluetooth.hpp>
#include <mqtt/Client.hpp>
#include <websocket/SolarControl.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace control
{

HTBluetooth::HTBluetooth(const string &ble_addr)
{
	auto mqtt = mqtt::Client::GetInstance();
	topic = "blegateway/" + ble_addr + "/sensor";
	mqtt->Subscribe(topic, this);
}

HTBluetooth::~HTBluetooth()
{
	auto mqtt = mqtt::Client::GetInstance();
	mqtt->Unsubscribe(topic, this);
}

double HTBluetooth::GetTemperature() const
{
	unique_lock<mutex> llock(lock);

	return temperature;
}

double HTBluetooth::GetHumidity() const
{
	unique_lock<mutex> llock(lock);

	return humidity;
}

void HTBluetooth::HandleMessage(const string &message)
{
	{
		unique_lock<mutex> llock(lock);

		try
		{
			json j = json::parse(message);
			humidity = j["humidity"];
			temperature = j["temperature"];
		}
		catch(json::exception &e)
		{
			return;
		}
	}

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
}

}


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

#include <sensor/weather/HTBluetooth.hpp>
#include <mqtt/Client.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::weather
{

HTBluetooth::HTBluetooth(const string &ble_addr)
{
	auto mqtt = mqtt::Client::GetInstance();
	topic = "sc-ble-gateway/" + ble_addr;
	mqtt->Subscribe(topic, this);
}

HTBluetooth::~HTBluetooth()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt)
		mqtt->Unsubscribe(topic, this);
}

void HTBluetooth::CheckConfig(const configuration::Json &conf)
{
	HT::CheckConfig(conf);

	conf.Check("ble_addr", "string");
	if(conf.GetString("ble_addr")=="")
		throw excpt::Config("Bluetooth address is mandatory", "ble_addr");
}

void HTBluetooth::HandleMessage(const string &message, const std::string & /*topic*/)
{
	{
		try
		{
			json j = json::parse(message);
			string payload = j["fcd2"];
			if(payload.size()!=20)
				return;

			humidity = (double)strtol(payload.substr(12, 2).c_str(), 0, 16);
			short utemp = (short)((strtol(payload.substr(18, 2).c_str(), 0, 16) << 8) + strtol(payload.substr(16, 2).c_str(), 0, 16)); // Temperature is a 16 bits signed integer
			temperature = (double)utemp / 10.0; // Convert to double

			notify_observer();
		}
		catch(json::exception &e)
		{
			return;
		}
	}
}

}


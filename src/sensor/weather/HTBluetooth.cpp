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
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>
#include <excpt/Config.hpp>

using namespace std;
using nlohmann::json;

namespace sensor::weather
{

HTBluetooth::HTBluetooth(const string &name,const string &ble_addr):HT(name)
{
	mqtt_topics = {"sc-ble-gateway/" + ble_addr};
}

HTBluetooth::~HTBluetooth()
{
}

void HTBluetooth::CheckConfig(const configuration::Json &conf)
{
	HT::CheckConfig(conf);

	conf.Check("ble_addr", "string");
	if(conf.GetString("ble_addr")=="")
		throw excpt::Config("Bluetooth address is mandatory", "ble_addr");
}

bool HTBluetooth::HandleMQTTMessage(const string &message, const std::string & /*topic*/)
{
	try
	{
		json j = json::parse(message);
		if(j.contains("fcd2")) // Shelly sensor
		{
			string payload = j["fcd2"];
			if(payload.size()!=20)
				return false;

			humidity = (double)strtol(payload.substr(12, 2).c_str(), 0, 16);
			short utemp = (short)((strtol(payload.substr(18, 2).c_str(), 0, 16) << 8) + strtol(payload.substr(16, 2).c_str(), 0, 16)); // Temperature is a 16 bits signed integer
			temperature = (double)utemp / 10.0; // Convert to double
		}
		else if(j.contains("181a")) // Custom sensor
		{
			string payload = j["181a"];
			if(payload.size()!=8)
				return false;

			long ihumidity = (strtol(payload.substr(4, 2).c_str(), 0, 16)<<8) | strtol(payload.substr(6, 2).c_str(), 0, 16);
			humidity = (double)ihumidity / 10;
			temperature = 0;
		}
	}
	catch(json::exception &e)
	{
		return false;
	}

	return true;
}

}


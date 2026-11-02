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

#include <shelly/AutodetectBLE.hpp>
#include <mqtt/Client.hpp>
#include <device/Devices.hpp>
#include <device/weather/HTBluetooth.hpp>

using namespace std;
using nlohmann::json;

namespace shelly {

AutodetectBLE::AutodetectBLE()
{
}

AutodetectBLE::~AutodetectBLE()
{
	auto mqtt = mqtt::Client::GetInstance();
	if(mqtt && mqtt_subscriber)
		mqtt->Unsubscribe("#", mqtt_subscriber);
}

void AutodetectBLE::DispatchMQTTMessage(const mqtt::st_message &mqtt_message)
{
	if(mqtt_message.topic.substr(0, 15)!="sc-ble-gateway/")
		return;

	string addr = mqtt_message.topic.substr(15);

	for(auto device : device::Devices::Get<device::HTBluetooth>())
	{
		if(device->GetBLEAddr()==addr)
			return; // Ignore already known devices
	}

	unique_lock<mutex> llock(wait_lock);
	ble_detected.notify_one();

	j_res = addr;
}

nlohmann::json AutodetectBLE::GetDevice()
{
	unique_lock<mutex> llock(wait_lock);

	mqtt_subscriber = make_shared<AutodetectBLESubscriber>(shared_from_this());
	mqtt::Client::GetInstance()->Subscribe("#", mqtt_subscriber);

	ble_detected.wait_for(llock, chrono::seconds(5));

	return j_res;
}

}



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

#include <mqtt/Client.hpp>
#include <configuration/ConfigurationSolarControl.hpp>

using namespace std;

namespace mqtt {

Client::Client(const string &host, unsigned int port, const string &topic)
{
	string id = configuration::ConfigurationSolarControl::GetInstance()->Get("mqtt.id");
	mosqh = mosquitto_new(id.c_str(), true, this);

	mosquitto_message_callback_set(mosqh, message_callback);

	int re = mosquitto_connect(mosqh, host.c_str(), port, 60);
	if(re!=MOSQ_ERR_SUCCESS)
		throw runtime_error("Could connect to host " + host + "on port " + to_string(port));

	mosquitto_subscribe(mosqh, 0, topic.c_str(), 0);

	loop_handle = thread(loop, this);
}

Client::~Client()
{
	mosquitto_destroy(mosqh);
}

void Client::Shutdown()
{
	mosquitto_disconnect(mosqh);
}

void Client::WaitForShutdown()
{
	loop_handle.join();
}

void Client::message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	Client *mqtt = (Client *)obj;
	mqtt->handle_message((const char *)message->payload);
}

void Client::loop(Client *mqtt)
{
	mosquitto_loop_forever(mqtt->mosqh, 1000, 1);
}

}

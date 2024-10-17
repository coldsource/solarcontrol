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

#ifndef __MQTT_MQTTCLIENT_HPP__
#define __MQTT_MQTTCLIENT_HPP__

#include <mosquitto.h>

#include <string>
#include <map>
#include <thread>
#include <mutex>

namespace mqtt {

class Subscriber;

class Client
{
	struct mosquitto *mosqh;

	std::thread loop_handle;
	std::mutex lock;

	std::map<std::string, Subscriber *> subscribers;

	static void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
	static void loop(Client *mqtt);

	static Client *instance;

	public:
		Client(const std::string &host, unsigned int port);
		~Client();

		static Client *GetInstance() { return instance; }

		void Subscribe(const std::string &topic, Subscriber *subscriber);

		void Shutdown();
		void WaitForShutdown();
};

}

#endif

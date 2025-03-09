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

#include <shelly/Autodetect.hpp>
#include <control/HTTP.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdexcept>
#include <regex>

using namespace std;
using nlohmann::json;

namespace shelly {

Autodetect::Autodetect() {}

json Autodetect::GetDevices(const string &host)
{
	j_devices = json::array();

	string ip = resolve(host);
	string network = ip_to_network(ip);

	for(int i=1; i<255; i++)
		workers.push_back(thread(worker, this, network + to_string(i)));

	for(size_t i=0; i<workers.size(); i++)
		workers[i].join();

	return j_devices;
}

string Autodetect::resolve(const string &host)
{
	struct addrinfo hints, *result;
	memset (&hints, 0, sizeof (hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;

	if(getaddrinfo(host.c_str(), 0, &hints, &result)!=0)
		throw runtime_error("Could not resolve host « " + host + " »");

	if(result->ai_family!=AF_INET)
	{
		freeaddrinfo(result);
		throw runtime_error("Autodetect only works in IPv4");
	}

	void *ptr = &((struct sockaddr_in *) result->ai_addr)->sin_addr;

	char addrstr[32];
	inet_ntop (result->ai_family, ptr, addrstr, 32);

	freeaddrinfo(result);
	return string(addrstr);
}

string Autodetect::ip_to_network(const string &ip)
{
	static regex regex_ip("([0-9]+\\.){3,3}");
	smatch matches;

	regex_search(ip, matches, regex_ip);
	if(matches.size()!=2)
		throw runtime_error("Not well formed IP « " + ip + " »");

	return matches[0].str();
}


void Autodetect::worker(Autodetect *instance, const string &ip)
{
	control::HTTP http(ip);

	try
	{
		json j;
		j["id"] = 1;
		j["method"] = "Shelly.GetDeviceInfo";

		auto res = http.Post(j);

		unique_lock<mutex> llock(instance->lock);
		json j_device;
		j_device["ip"] = ip;
		j_device["type"] = res["result"]["app"];
		j_device["name"] = res["result"]["name"];
		instance->j_devices.push_back(j_device);
	}
	catch(exception &e) {}
}

}


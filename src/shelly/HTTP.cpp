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

#include <shelly/HTTP.hpp>
#include <configuration/Configuration.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <excpt/Shelly.hpp>

#include <curl/curl.h>

using nlohmann::json;
using namespace std;

namespace shelly {

HTTP::HTTP(const std::string &ip)
{
	this-> ip = ip;
}

json HTTP::Post(const json &j) const
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();

	try
	{
		// Configure HTTP timeout
		unsigned long timeout = configuration::Configuration::FromType("solarcontrol")->GetTime("http.timeout");
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT , timeout);

		curl_easy_setopt(curl, CURLOPT_URL, ("http://" + ip + "/rpc").c_str());

		string param = j.dump();
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, param.c_str());

		string output;
		auto write_cbk = +[](char *ptr, size_t size, size_t nmemb, void *userdata) {
			((string *)userdata)->append(string(ptr, size * nmemb));
			return (size_t)size * nmemb;
		};

		curl_easy_setopt(curl, CURLOPT_WRITEDATA , &output);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cbk);


		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
			throw excpt::Shelly("Error executing HTTP API Command on « " + ip + " »");

		int http_code;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if(http_code!=200)
			throw excpt::Shelly("Error executing HTTP API Command on « " + ip + " » got " + to_string(http_code) + " http response code");

		auto output_j = json::parse(output);
		if(output_j.contains("error"))
			throw excpt::Shelly("Error executing HTTP API Command on « " + ip + " » got error " + "« " + string(output_j["error"]["message"]) + " »");

		curl_easy_cleanup(curl);

		return output_j;
	}
	catch(exception &e)
	{
		curl_easy_cleanup(curl);
		throw;
	}
}

}

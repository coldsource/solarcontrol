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

HTTP::HTTP(const std::string &ip): http::HTTP(ip)
{
}

json HTTP::Post(const json &j) const
{
	string output;
	json output_j;
	try
	{
		output = http::HTTP::Post("/rpc", j.dump());
		output_j = json::parse(output);
	}
	catch(exception &e)
	{
		throw excpt::Shelly(e.what());
	}

	if(output_j.contains("error"))
		throw excpt::Shelly("Error executing HTTP API Command on « " + ip + " » got error " + "« " + string(output_j["error"]["message"]) + " »");

	return output_j;
}

}

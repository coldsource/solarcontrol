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

#include <shelly/Sys.hpp>
#include <control/HTTP.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;

namespace shelly {

json Sys::Get()
{
	control::HTTP http(ip);

	json j;
	j["id"] = 1;
	j["method"] = "Sys.GetConfig";

	return http.Post(j)["result"];
}

json Sys::Set(const string &name)
{
	control::HTTP http(ip);

	json j;
	j["id"] = 1;
	j["method"] = "Sys.SetConfig";

	json params;
	params["config"] = json::object();
	params["config"]["device"] = json::object();
	params["config"]["device"]["name"] = name;
	j["params"] = params;

	return http.Post(j)["result"];
}

json Sys::Reboot()
{
	control::HTTP http(ip);

	json j;
	j["id"] = 1;
	j["method"] = "Shelly.Reboot";

	return http.Post(j)["result"];
}

}




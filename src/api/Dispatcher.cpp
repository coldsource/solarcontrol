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

#include <api/Dispatcher.hpp>
#include <api/DeviceElectrical.hpp>
#include <api/DeviceHT.hpp>
#include <api/Logs.hpp>
#include <api/Config.hpp>
#include <api/Storage.hpp>
#include <api/Shelly.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;

namespace api
{

Dispatcher::Dispatcher()
{
	handlers["deviceelectrical"] = new DeviceElectrical();
	handlers["deviceht"] = new DeviceHT();
	handlers["logs"] = new Logs();
	handlers["config"] = new Config();
	handlers["storage"] = new Storage();
	handlers["shelly"] = new Shelly();
}

Dispatcher::~Dispatcher()
{
	for(auto it = handlers.begin(); it!=handlers.end(); ++it)
		delete it->second;
}

string Dispatcher::Dispatch(const std::string &message)
{
	configuration::Json api;

	if(message=="") // Keepalive from JS
		return "";

	try
	{
		api = configuration::Json(message);
	}
	catch(exception &e)
	{
		throw invalid_argument("Invalid API command : not valid json");
	}

	unsigned int id = api.GetInt("id");
	string module = api.GetString("module");
	string cmd = api.GetString("cmd");

	auto handler = handlers.find(module);
	if(handler==handlers.end())
		throw invalid_argument("Uknown module « " + module + " »" );

	configuration::Json params;
	if(api.Has("parameters"))
		params = api.GetObject("parameters");

	auto j_res = json();
	j_res["id"] = id;

	try
	{
		j_res["res"] = handler->second->HandleMessage(cmd, params);
		j_res["status"] = "ok";

		return j_res.dump();
	}
	catch(exception &e)
	{
		j_res["status"] = "error";
		j_res["message"] = string(e.what());

		return j_res.dump();
	}
}

}

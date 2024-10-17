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
#include <api/Device.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;

namespace api
{

Dispatcher::Dispatcher()
{
	handlers["device"] = new Device();
}

Dispatcher::~Dispatcher()
{
	for(auto it = handlers.begin(); it!=handlers.end(); ++it)
		delete it->second;
}

string Dispatcher::Dispatch(const std::string &message)
{
	json j;

	try
	{
		j = json::parse(message);
	}
	catch(exception &e)
	{
		throw invalid_argument("Invalid API command : not valid json");
	}

	if(!j.contains("module"))
		throw invalid_argument("Missing « module » in API command");

	if(!j.contains("cmd"))
		throw invalid_argument("Missing « cmd » in API command");

	auto handler = handlers.find(j["module"]);
	if(handler==handlers.end())
		throw invalid_argument("Uknown module « " + string(j["module"]) + " »" );

	json j_params;
	if(j.contains("parameters"))
		j_params = j["parameters"];

	auto j_res = handler->second->HandleMessage(j["cmd"], j_params);
	return j_res.dump();
}

}
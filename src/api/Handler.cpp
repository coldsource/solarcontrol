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

#include <api/Handler.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;

namespace api {

void Handler::check_param(const json &j, const string &name, const string &type)
{
	if(!j.contains(name))
		throw invalid_argument("Missing parameter « " + name + " »");

	auto jtype = j[name].type();
	if(type=="int" && jtype!=json::value_t::number_unsigned)
		throw invalid_argument("Parameter « " + name + " » must be integer");
	else if(type=="unsigned int" && jtype!=json::value_t::number_unsigned)
		throw invalid_argument("Parameter « " + name + " » must be unsigned integer");
	else if(type=="float" && jtype!=json::value_t::number_float)
		throw invalid_argument("Parameter « " + name + " » must be float");
	else if(type=="string" && jtype!=json::value_t::string)
		throw invalid_argument("Parameter « " + name + " » must be string");
	else if(type=="bool" && jtype!=json::value_t::boolean)
		throw invalid_argument("Parameter « " + name + " » must be boolean");
	else if(type=="array" && jtype!=json::value_t::array)
		throw invalid_argument("Parameter « " + name + " » must be array");
	else if(type=="object" && jtype!=json::value_t::object)
		throw invalid_argument("Parameter « " + name + " » must be object");

	if(type=="string" && j[name]=="")
		throw invalid_argument("« " + name + " » can't be empty");
}

}

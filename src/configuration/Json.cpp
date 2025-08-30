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

#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;

namespace configuration
{

Json::Json(const string &json_str)
{
	try
	{
		json = json::parse(json_str);
	}
	catch(...)
	{
		throw invalid_argument("Invalid json configuration string, not valid json");
	}
}

void Json::check_entry(const string &name, const string &type) const
{
	if(!json.contains(name))
		throw invalid_argument("Missing parameter « " + name + " »");

	auto jtype = json[name].type();
	if(type=="int" && jtype!=json::value_t::number_integer && jtype!=json::value_t::number_unsigned)
		throw invalid_argument("Parameter « " + name + " » must be integer");
	if(type=="uint" && jtype!=json::value_t::number_unsigned)
		throw invalid_argument("Parameter « " + name + " » must be integer");
	else if(type=="unsigned int" && jtype!=json::value_t::number_unsigned)
		throw invalid_argument("Parameter « " + name + " » must be unsigned integer");
	else if(type=="float" && jtype!=json::value_t::number_float && jtype!=json::value_t::number_integer && jtype!=json::value_t::number_unsigned)
		throw invalid_argument("Parameter « " + name + " » must be float");
	else if(type=="string" && jtype!=json::value_t::string)
		throw invalid_argument("Parameter « " + name + " » must be string");
	else if(type=="bool" && jtype!=json::value_t::boolean)
		throw invalid_argument("Parameter « " + name + " » must be boolean");
	else if(type=="array" && jtype!=json::value_t::array)
		throw invalid_argument("Parameter « " + name + " » must be array");
	else if(type=="object" && jtype!=json::value_t::object)
		throw invalid_argument("Parameter « " + name + " » must be object");
}

void Json::Check(const std::string &name, const std::string &type, bool required) const
{
	if(!required && !json.contains(name))
		return;

	check_entry(name, type);
}

bool Json::Has(const std::string &name) const
{
	return json.contains(name);
}

string Json::GetString(const string &name) const
{
	check_entry(name, "string");
	return json[name];
}

string Json::GetString(const string &name, const std::string &default_value) const
{
	if(!json.contains(name))
		return default_value;

	return GetString(name);
}

int Json::GetInt(const string &name) const
{
	check_entry(name, "int");
	return json[name];
}

int Json::GetInt(const string &name,int default_value) const
{
	if(!json.contains(name))
		return default_value;

	return GetInt(name);
}

unsigned int Json::GetUInt(const string &name) const
{
	check_entry(name, "uint");
	return json[name];
}

unsigned int Json::GetUInt(const string &name, unsigned int default_value) const
{
	if(!json.contains(name))
		return default_value;

	return GetUInt(name);
}

double Json::GetFloat(const std::string &name) const
{
	check_entry(name, "float");
	return json[name];
}

double Json::GetFloat(const std::string &name, double default_value) const
{
	if(!json.contains(name))
		return default_value;

	return GetFloat(name);
}

bool Json::GetBool(const std::string &name) const
{
	check_entry(name, "bool");
	return json[name];
}

bool Json::GetBool(const std::string &name, bool default_value) const
{
	if(!json.contains(name))
		return default_value;

	return GetBool(name);
}

const Json Json::GetArray(const string &name) const
{
	check_entry(name, "array");
	return Json(json[name]);
}

const Json Json::GetArray(const string &name, const nlohmann::json &default_value) const
{
	if(!json.contains(name))
		return default_value;

	return GetArray(name);
}

const Json Json::GetObject(const string &name) const
{
	check_entry(name, "object");
	return Json(json[name]);
}

Json::Iterator& Json::Iterator::operator++()
{
	pos++;
	return *this;
}

bool Json::Iterator::operator==(const Iterator& r) const
{
	return (pos==r.pos && conf_ptr==r.conf_ptr);
}

Json Json::Iterator::operator*() const
{
	return Json(conf_ptr->json.at(pos));
}

Json::Iterator Json::begin() const
{
	if(json.type()!=json::value_t::array)
		throw invalid_argument("Could not iterate over non array");

	return Iterator(this, 0);
}

Json::Iterator Json::end() const
{
	if(json.type()!=json::value_t::array)
		throw invalid_argument("Could not iterate over non array");

	return Iterator(this,json.size());
}

size_t Json::size() const
{
	if(json.type()!=json::value_t::array)
		throw invalid_argument("Could not get size of non array");

	return json.size();
}

}

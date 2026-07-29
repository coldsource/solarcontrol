/*
 * This file is part of evQueue
 *
 * evQueue is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * evQueue is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with evQueue. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Thibault Kummer <bob@coldsource.net>
 */

#ifndef  __CONFIGURATION_UNITS_H__
#define  __CONFIGURATION_UNITS_H__

#include <string>
#include <map>
#include <stdexcept>
#include <algorithm>

namespace configuration
{

template<typename T>
T decode_unit_value(const std::string &value, std::map<std::string, T> units, bool signed_value = false)
{
	size_t l;
	T val;

	try
	{
		if(std::is_same<T, float>::value)
			val = stof(value, &l);
		else if(std::is_same<T, double>::value)
			val = stod(value, &l);
		else if(std::is_same<T, int>::value)
			val = stoi(value, &l);
		else if(std::is_same<T, unsigned int>::value)
			val = (unsigned int)stoi(value, &l);
		else if(std::is_same<T, long>::value)
			val = stol(value, &l);
		else if(std::is_same<T, unsigned long>::value)
			val = stoul(value, &l);
		else if(std::is_same<T, long long>::value)
			val = stoll(value, &l);
		else if(std::is_same<T, unsigned long long>::value)
			val = stoull(value, &l);
		else
			throw std::logic_error("decode_unit_value: Unable to decode data type");
	}
	catch(std::invalid_argument &e)
	{
		throw std::runtime_error("Invalid numerical value");
	}
	catch(std::out_of_range &e)
	{
		throw std::runtime_error("Value too big");
	}

	if(!signed_value && val<0)
		throw std::runtime_error("Negative value is not allowed");

	if(l==value.length())
		return val; // Numerical value only (no units)

	std::string unit = value.substr(l);
	std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
	auto it_unit = units.find(unit);
	if(it_unit==units.end())
		throw std::runtime_error("Unknown unit « " + unit + " »");

	return val * (it_unit->second);
}

}

#endif


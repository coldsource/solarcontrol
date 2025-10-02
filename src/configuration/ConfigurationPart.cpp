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

#include <configuration/ConfigurationPart.hpp>
#include <configuration/Configuration.hpp>
#include <websocket/SolarControl.hpp>
#include <excpt/Config.hpp>

#include <regex>
#include <stdexcept>
#include <algorithm>

#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

using namespace std;

namespace configuration
{

ConfigurationPart::ConfigurationPart(void)
{
}

ConfigurationPart::ConfigurationPart(const map<string,string> &entries)
{
	this->entries = entries;
}

void ConfigurationPart::notify_observers()
{
	Configuration::GetInstance()->notify_observers(this);
}

bool ConfigurationPart::Set(const string &entry,const string &value)
{
	{
		unique_lock<recursive_mutex> llock(lock);

		if(entries.count(entry)==0)
			return false;

		entries[entry] = value;
	}

	// Unlock before notifying observers
	notify_observers();

	return true;
}

bool ConfigurationPart::SetCheck(const string &entry,const string &value)
{
	{
		unique_lock<recursive_mutex> llock(lock);

		if(entries.count(entry)==0)
			return false;

		string old_value = entries[entry];
		entries[entry] = value;

		try
		{
			Check();
		}
		catch(exception &e)
		{
			entries[entry] = old_value;
			throw;
		}
	}

	// Unlock before notifying observers
	notify_observers();

	return true;
}

const string &ConfigurationPart::Get(const string &entry) const
{
	unique_lock<recursive_mutex> llock(lock);

	map<string,string>::const_iterator it = entries.find(entry);
	if(it==entries.end())
		throw excpt::Config("Unknown configuration entry: " + entry, entry);
	return it->second;
}

template<typename T>
T ConfigurationPart::decode_unit_value(const string &value, map<string, T> units, bool signed_value) const
{
	size_t l;
	T val;

	try
	{
		if(is_same<T, float>::value)
			val = stof(value, &l);
		else if(is_same<T, double>::value)
			val = stod(value, &l);
		else if(is_same<T, int>::value)
			val = stoi(value, &l);
		else if(is_same<T, unsigned int>::value)
			val = (unsigned int)stoi(value, &l);
		else if(is_same<T, long>::value)
			val = stol(value, &l);
		else if(is_same<T, unsigned long>::value)
			val = stoul(value, &l);
		else if(is_same<T, long long>::value)
			val = stoll(value, &l);
		else if(is_same<T, unsigned long long>::value)
			val = stoull(value, &l);
		else
			throw logic_error("decode_unit_value: Unable to decode data type");
	}
	catch(invalid_argument &e)
	{
		throw runtime_error("Invalid numerical value");
	}
	catch(out_of_range &e)
	{
		throw runtime_error("Value too big");
	}

	if(!signed_value && val<0)
		throw runtime_error("Negative value is not allowed");

	if(l==value.length())
		return val; // Numerical value only (no units)

	string unit = value.substr(l);
	std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
	auto it_unit = units.find(unit);
	if(it_unit==units.end())
		throw runtime_error("Unknown unit « " + unit + " »");

	return val * (it_unit->second);
}

int ConfigurationPart::GetInt(const string &entry, bool signed_int) const
{
	const string value = Get(entry);

	int val;
	size_t l;
	try
	{
		if(value.substr(0,2)=="0x")
		{
			val = stoi(value.substr(2), &l, 16);
			if(l + 2 != value.length())
				throw excpt::Config(entry, value, "Invalid hexadecimal value");
		}
		else
		{
			val = stoi(value, &l, 10);

			if(l != value.length())
				throw excpt::Config(entry, value, "Invalid decimal value");
		}
	}
	catch(invalid_argument &e)
	{
		throw excpt::Config(entry, value, "Invalid numerical value");
	}
	catch(out_of_range &e)
	{
		throw excpt::Config(entry, value, "Value too big");
	}

	if(!signed_int && val<0)
		throw excpt::Config(entry, value, "Negative value is not allowed");

	return val;
}

unsigned int ConfigurationPart::GetUInt(const string &entry) const
{
	return (unsigned int)GetInt(entry);
}

double ConfigurationPart::GetDouble(const string &entry, bool signed_double) const
{
	const string value = Get(entry);

	try
	{
		return decode_unit_value<double>(value, {}, signed_double);
	}
	catch(exception &e)
	{
		throw excpt::Config(entry, value, e.what());
	}
}

unsigned long ConfigurationPart::GetTime(const string &entry) const
{
	const string value = Get(entry);

	try
	{
		return decode_unit_value<unsigned long>(value, {{"d", 86400},  {"h", 3600}, {"m", 60}, {"s", 1}});
	}
	catch(exception &e)
	{
		throw excpt::Config(entry, value, e.what());
	}
}

int ConfigurationPart::GetPower(const string &entry, bool signed_int) const
{
	const string value = Get(entry);

	try
	{
		return decode_unit_value<int>(value, {{"w", 1},  {"kw", 1000}}, signed_int);
	}
	catch(exception &e)
	{
		throw excpt::Config(entry, value, e.what());
	}
}

int ConfigurationPart::GetEnergy(const string &entry, bool signed_int) const
{
	const string value = Get(entry);

	try
	{
		return decode_unit_value<int>(value, {{"wh", 1},  {"kwh", 1000}}, signed_int);
	}
	catch(exception &e)
	{
		throw excpt::Config(entry, value, e.what());
	}
}

double ConfigurationPart::GetPercent(const string &entry) const
{
	const string value = Get(entry);

	try
	{
		return decode_unit_value<double>(value, {{"%", 1}}) / 100;
	}
	catch(exception &e)
	{
		throw excpt::Config(entry, value, e.what());
	}
}

bool ConfigurationPart::GetBool(const string &entry) const
{
	const string value = Get(entry);
	if(value=="yes" || value=="true" || value=="1")
		return true;
	return false;
}


bool ConfigurationPart::Exists(const std::string &name) const
{
	return entries.contains(name);
}

void ConfigurationPart::check_bool_entry(const string &name) const
{
	const string value = Get(name);

	if(value=="yes" || value=="true" || value=="1")
		return;

	if(value=="no" || value=="false" || value=="0")
		return;

	throw excpt::Config(name, value, "Invalid boolean value");
}

void ConfigurationPart::check_int_entry(const string &name, bool signed_int) const
{
	GetInt(name, signed_int);
}

void ConfigurationPart::check_double_entry(const string &name, bool signed_double) const
{
	GetDouble(name, signed_double);
}

void ConfigurationPart::check_time_entry(const string &name) const
{
	GetTime(name);
}

void ConfigurationPart::check_power_entry(const string &name, bool signed_int) const
{
	GetPower(name, signed_int);
}

void ConfigurationPart::check_energy_entry(const string &name, bool signed_int) const
{
	GetEnergy(name, signed_int);
}

void ConfigurationPart::check_percent_entry(const string &name) const
{
	GetPercent(name);
}

void ConfigurationPart::Backup(const string &name)
{
	backups[name] = entries;
}

const map<string, string> ConfigurationPart::GetBackup(const string &name)
{
	auto it = backups.find(name);
	if(it==backups.end())
		throw runtime_error("Unknown backup « " + name + " »");

	return it->second;
}

}


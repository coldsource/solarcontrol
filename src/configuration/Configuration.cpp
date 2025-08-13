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

#include <configuration/Configuration.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <configuration/ConfigurationObserver.hpp>
#include <websocket/SolarControl.hpp>

#include <regex>
#include <stdexcept>

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

unique_ptr<Configuration> Configuration::instance;

Configuration::Configuration(void)
{
}

Configuration::~Configuration(void)
{
}

Configuration *Configuration::GetInstance()
{
	if(instance==nullptr)
	{
		instance = make_unique<Configuration>();
	}

	return instance.get();
}

ConfigurationPart *Configuration::FromType(const std::string &type)
{
	auto it = instance->configs.find(type);
	if(it==instance->configs.end())
		throw runtime_error("Unknown configuration type : « " + type + " »");

	return it->second.get();
}

// Add a new part to the global configuration
bool Configuration::RegisterConfig(shared_ptr<ConfigurationPart> config)
{
	string type = config->GetType();

	if(configs.contains(type))
		throw runtime_error("Duplicated configuration part : « " + type + " »");

	// Store configuration pointer
	configs[type] = config;

	// Register all configuration entries
	for(auto entry : config->entries)
	{
		if(entry_type.contains(entry.first))
			throw runtime_error("Duplicated configuration entry : « " + entry.first + " »");

		entry_type[entry.first] = type;
	}

	return true;
}

bool Configuration::Set(const std::string &name,const std::string &value)
{
	// Check entry exists in some part
	auto entry = entry_type.find(name);
	if(entry==entry_type.end())
		return false;

	// Forward new configuration value to specific part
	string type = entry->second;
	configs[type]->Set(name, value);
	return true;
}

void Configuration::Backup(const std::string &name)
{
	for(auto config : configs)
		config.second->Backup(name);
}

void Configuration::RegisterObserver(const string &type, ConfigurationObserver *observer)
{
	unique_lock<recursive_mutex> llock(lock);

	if(!configs.contains(type))
		throw runtime_error("Unknown configuration type : « " + type + " »");

	observer->ConfigurationChanged(configs[type].get());
	observers[type].insert(observer);
}

void Configuration::UnregisterObserver(const string &type, ConfigurationObserver *observer)
{
	unique_lock<recursive_mutex> llock(lock);

	if(!configs.contains(type))
		throw runtime_error("Unknown configuration type : « " + type + " »");

	observers[type].erase(observer);
}

void Configuration::notify_observers(ConfigurationPart *part)
{
	string type = part->GetType();

	for(auto observer : observers[type])
		observer->ConfigurationChanged(part);

	if(websocket::SolarControl::GetInstance())
		websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::CONFIG);
}

void Configuration::Check()
{
	for(auto config : configs)
		config.second->Check();
}

}

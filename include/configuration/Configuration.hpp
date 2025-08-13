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

#ifndef  __CONFIGURATION_CONFIGURATION_H__
#define  __CONFIGURATION_CONFIGURATION_H__

#include <string>
#include <map>
#include <set>
#include <mutex>
#include <memory>

namespace configuration
{

class ConfigurationObserver;
class ConfigurationPart;

class Configuration
{
	friend class ConfigurationPart;

	private:
		std::map<std::string, std::shared_ptr<ConfigurationPart>> configs;
		std::map<std::string, std::string> entry_type;

		std::map<std::string, std::set<ConfigurationObserver *>> observers;

		mutable std::recursive_mutex lock;

		void notify_observers(ConfigurationPart *part);

	protected:
		static std::unique_ptr<Configuration> instance;

	public:
		Configuration(void);
		virtual ~Configuration(void);

		static Configuration *GetInstance();
		static ConfigurationPart *FromType(const std::string &type);

		bool RegisterConfig(std::shared_ptr<ConfigurationPart> config);

		bool Set(const std::string &name,const std::string &value);
		void Backup(const std::string &name);

		void RegisterObserver(const std::string &type, ConfigurationObserver *observer);
		void UnregisterObserver(const std::string &type, ConfigurationObserver *observer);

		void Check(void);
};

}

#endif


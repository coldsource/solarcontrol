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

#ifndef  __CONFIGURATION_CONFIGURATIONPART_H__
#define  __CONFIGURATION_CONFIGURATIONPART_H__

#include <string>
#include <map>
#include <mutex>

namespace configuration
{

class Configuration;

class ConfigurationPart
{
	friend class Configuration;

	protected:
		mutable std::recursive_mutex lock;
		std::map<std::string,std::string> entries;
		std::map<std::string, std::map<std::string,std::string>> backups;

		void notify_observers();

	public:
		ConfigurationPart(void);
		ConfigurationPart(const std::map<std::string, std::string> &entries);
		virtual ~ConfigurationPart(void) {}

		virtual std::string GetType() const = 0;

		virtual void Check(void) const {}

		bool Set(const std::string &entry,const std::string &value);
		bool SetCheck(const std::string &entry,const std::string &value);
		const std::map<std::string,std::string> GetAll() const { return entries; }
		const std::string &Get(const std::string &entry) const;
		int GetInt(const std::string &entry) const;
		double GetDouble(const std::string &entry) const;
		int GetSize(const std::string &entry) const;
		int GetTime(const std::string &entry) const;
		int GetPower(const std::string &entry) const;
		int GetEnergy(const std::string &entry) const;
		int GetPercent(const std::string &entry) const;
		bool GetBool(const std::string &entry) const;
		int GetUID(const std::string &entry) const;
		int GetGID(const std::string &entry) const;
		bool Exists(const std::string &name) const;

		void Backup(const std::string &name);
		const std::map<std::string,std::string> GetBackup(const std::string &name);

	protected:
		void check_f_is_exec(const std::string &filename) const;
		void check_d_is_writeable(const std::string &path) const;
		void check_bool_entry(const std::string &name) const;
		void check_int_entry(const std::string &name, bool signed_int=false) const;
		void check_double_entry(const std::string &name, bool signed_int=false) const;
		void check_size_entry(const std::string &name) const;
		void check_time_entry(const std::string &name) const;
		void check_power_entry(const std::string &name, bool signed_int=false) const;
		void check_energy_entry(const std::string &name, bool signed_int=false) const;
		void check_percent_entry(const std::string &name) const;
};

}

#endif

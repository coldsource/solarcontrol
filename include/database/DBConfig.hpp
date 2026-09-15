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

#ifndef __DATABASE_DBCONFIG_H__
#define __DATABASE_DBCONFIG_H__

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace database {

class DBConfig
{
	std::map<std::string, std::string> tables;
	std::map<std::string, std::vector<std::string>> tables_data;

	static std::unique_ptr<DBConfig> instance;
	
	public:
		static DBConfig *GetInstance();
		
		bool RegisterTables(std::map<std::string, std::string> &tables_def, std::map<std::string, std::vector<std::string>> &tables_data_def);
		void InitTables();
};

}

#endif

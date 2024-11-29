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

#include <database/DBConfig.hpp>
#include <database/DB.hpp>
#include <configuration/ConfigurationSolarControl.hpp>
#include <logs/Logger.hpp>

using namespace std;

namespace database {

DBConfig *DBConfig::instance = 0;

DBConfig *DBConfig::GetInstance()
{
	if(instance == 0)
		instance = new DBConfig();
	
	return instance;
}

bool DBConfig::RegisterTables(map<string, string> &tables_def)
{
	// Merge new tables in existing scheme
	for(auto it = tables_def.begin(); it!=tables_def.end(); ++it)
		tables[it->first] = it->second;

	return true;
}

void DBConfig::InitTables()
{
	string database = configuration::ConfigurationSolarControl::GetInstance()->Get("sql.database");

	// Init tables
	DB db;
	for(auto it_table=tables.begin();it_table!=tables.end();++it_table)
	{
		auto res = db.Query(
			"SELECT table_comment FROM INFORMATION_SCHEMA.TABLES WHERE table_schema=%s AND table_name=%s"_sql<<database<<it_table->first
		);

		if(!res.FetchRow())
		{
			logs::Logger::Log(LOG_NOTICE, "Table " + it_table->first + " does not exists, creating it...");

			db.Query(it_table->second);
		}
	}
}

}

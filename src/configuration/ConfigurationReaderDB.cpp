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

#include <configuration/ConfigurationReaderDB.hpp>
#include <configuration/Configuration.hpp>
#include <database/DB.hpp>

using namespace std;

namespace configuration {

void ConfigurationReaderDB::Read(Configuration *config)
{
	auto configs = config->GetConfigs();

	database::DB db;
	auto res = db.Query("SELECT config_name, config_value FROM t_config"_sql);
	while(res.FetchRow())
	{
		for(size_t i=0; i<configs.size(); i++)
		{
			if(configs[i]->Exists(res["config_name"]))
				configs[i]->Set(res["config_name"], res["config_value"]);
		}
	}
}

}

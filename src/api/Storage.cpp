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

#include <api/Storage.hpp>
#include <database/DB.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using database::DB;

namespace api
{

json Storage::HandleMessage(const string &cmd, const configuration::Json &j_params)
{
	json j_res;

	if(cmd=="get")
	{
		string name = j_params.GetString("name");

		DB db;
		auto res = db.Query("SELECT storage_value FROM t_storage WHERE storage_name=%s"_sql<<name);
		while(res.FetchRow())
			j_res = json::parse(string(res["storage_value"]));

		return j_res;
	}
	else if(cmd=="set")
	{
		string name = j_params.GetString("name");
		auto value = j_params.GetObject("value");

		DB db;
		db.Query("REPLACE INTO t_storage(storage_name, storage_value) VALUES(%s, %s)"_sql<<name<<value.ToString());

		return json();
	}
	else if(cmd=="remove")
	{
		string name = j_params.GetString("name");

		DB db;
		db.Query("DELETE FROM t_storage WHERE storage_name=%s"_sql<<name);

		return json();
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « storage »");
}

}

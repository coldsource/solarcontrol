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

#include <upgrade/Upgrade.hpp>
#include <database/DB.hpp>

#include <regex>
#include <stdexcept>

using namespace std;

namespace upgrade {

string Upgrade::get_version() const
{
	database::DB db;

	auto res = db.Query("SELECT storage_value FROM t_storage WHERE storage_name='version'"_sql);
	if(!res.FetchRow())
		throw runtime_error("No version set in database");

	return res["storage_value"];
}

int Upgrade::VersionCompare(const std::string &v1, const std::string &v2)
{
	const static regex version_regex("^([0-9]+)\\.([0-9]+)$");

	smatch matches1, matches2;
	regex_search(v1, matches1, version_regex);
	regex_search(v2, matches2, version_regex);

	if(matches1.size()!=3 || matches2.size()!=3)
		throw runtime_error("Invalid version number");

	if(v1==v2)
		return 0;

	int v1_major = stoi(matches1[1].str());
	int v1_minor = stoi(matches1[2].str());
	int v2_major = stoi(matches2[1].str());
	int v2_minor = stoi(matches2[2].str());

	if(v1_major<v2_major || (v1_major==v2_major && v1_minor<v2_minor))
		return -1;

	return 1;
}

}

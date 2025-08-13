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

#include <database/DB.hpp>
#include <configuration/Configuration.hpp>
#include <configuration/ConfigurationPart.hpp>

#include <stdexcept>
#include <regex>

using namespace std;

namespace database
{

DB::DB()
{
	auto config = configuration::Configuration::FromType("solarcontrol");
	host = config->Get("sql.host");
	user = config->Get("sql.user");
	password = config->Get("sql.password");
	database = config->Get("sql.database");

	mysql = mysql_init(0);

	if(!mysql_real_connect(mysql, host.c_str(), user.c_str(), password.c_str(), database.c_str(), 0, 0, 0))
		throw runtime_error("SQL connection error " + to_string(mysql_errno(mysql)) + " : « " + string(mysql_error(mysql)) + " »");
}

DB::~DB()
{
	mysql_close(mysql);
}

void DB::InitLibrary(void)
{
	mysql_library_init(0,0,0);
}

void DB::FreeLibrary(void)
{
	mysql_library_end();
}

void DB::StartThread(void)
{
	mysql_thread_init();
}

void DB::StopThread(void)
{
	mysql_thread_end();
}

Result DB::Query(const database::Query &q)
{
	regex prct_regex("%(s|i|f)");

	string query = q.GetQuery();

	auto words_begin = sregex_iterator(query.begin(), query.end(), prct_regex);
	auto words_end = sregex_iterator();

	int last_pos = 0;
	int match_i = 0;
	string escaped_query;
	for (sregex_iterator i = words_begin; i != words_end; ++i)
	{
		smatch m = *i;
		escaped_query += query.substr(last_pos, m.position()-last_pos) + get_query_value(m.str()[1], match_i, q);
		last_pos = m.position()+m.length();

		match_i++;
	}

	escaped_query += query.substr(last_pos);

	if(mysql_query(mysql, escaped_query.c_str())!=0)
		throw runtime_error("SQL error " + to_string(mysql_errno(mysql)) + " « " + string(mysql_error(mysql)) + " » while executing query « " + escaped_query + " »");

	MYSQL_RES *res = mysql_store_result(mysql);
	if(res==0 && mysql_field_count(mysql)!=0)
		throw runtime_error("SQL error " + to_string(mysql_errno(mysql)) + " « " + string(mysql_error(mysql)) + " » while executing query « " + escaped_query + " »");

	return Result(res);
}

string DB::get_query_value(char type, int idx, const database::Query &q)
{
	auto param = q.GetParam(idx);

	if(param.type==Query::NULLVAL)
		return "NULL";

	switch(type)
	{
		case 's':
			if(param.type!=Query::STRING) throw invalid_argument("Invalid database parameter number " + to_string(idx) + ", expecting string");
			return "'" + EscapeString(param.val_string) + "'";
		case 'i':
			if(param.type!=Query::INT) throw invalid_argument("Invalid database parameter number " + to_string(idx) + ", expecting int");
			return to_string(param.val_int);
		case 'f':
			if(param.type!=Query::FLOAT) throw invalid_argument("Invalid database parameter number " + to_string(idx) + ", expecting float");
			return to_string(param.val_float);
	}

	return "";
}

string DB::EscapeString(const string &str)
{
	char *buf = new char[2*str.size()+1];
	long unsigned int size = mysql_real_escape_string(mysql, buf, str.c_str(), str.size());
	string escaped_str(buf, size);
	delete []buf;

	return escaped_str;
}

int DB::InsertID(void)
{
	return mysql_insert_id(mysql);
}

}

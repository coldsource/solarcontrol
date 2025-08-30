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

#ifndef __DB_HPP__
#define __DB_HPP__

#include <database/Query.hpp>
#include <database/Result.hpp>

#ifdef MYSQL_PATH_MYSQL
#include <mysql/mysql.h>
#endif

#ifdef MYSQL_PATH_MARIADB
#include <mariadb/mysql.h>
#endif

#include <string>

namespace database {

class DB
{
	std::string host;
	std::string user;
	std::string password;
	std::string database;

	MYSQL *mysql;

	public:
		DB();
		~DB();

		static void InitLibrary(void);
		static void FreeLibrary(void);

		static void StartThread(void);
		static void StopThread(void);

		Result Query(const Query &query);
		std::string get_query_value(char type, unsigned int idx, const database::Query &q);
		std::string EscapeString(const std::string &str);

		unsigned long long InsertID();
};

}

#endif


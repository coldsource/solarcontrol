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

#ifndef __RESULT_HPP__
#define __RESULT_HPP__

#ifdef MYSQL_PATH_MYSQL
#include <mysql/mysql.h>
#endif

#ifdef MYSQL_PATH_MARIADB
#include <mariadb/mysql.h>
#endif

#include <string>
#include <map>

namespace database {

class ResultField
{
	std::string value;

	public:
		ResultField(const std::string &value) { this->value = value; }

		operator std::string() const { return value; }
		operator int() const { return std::stoi(value); }
		operator unsigned int() const { return std::stoi(value); }
		operator double() const { return std::stod(value); }
};

class Result
{
	MYSQL_RES *res;

	unsigned int cols;
	std::map<std::string, int> col_name_idx;

	MYSQL_ROW row;
	unsigned long *row_field_length;

	public:
		Result(MYSQL_RES *res);
		Result(const Result &) = delete;
		~Result();

		bool FetchRow(void);
		std::string GetField(unsigned int n) const;

		const ResultField operator[](unsigned int n);
		const ResultField operator[](const std::string &name);
};

}

#endif


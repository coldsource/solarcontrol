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

#ifndef __QUERY_HPP__
#define __QUERY_HPP__

#include <string>
#include <vector>

namespace database {

class Query
{
	private:
		std::string query;

	public:
		enum en_query_param_type {STRING, INT, FLOAT};
		struct st_query_param
		{
			en_query_param_type type;
			std::string val_string;
			int val_int = 0;
			double val_float = 0;
		};

	private:
		std::vector<st_query_param> params;

	public:
		Query() {}
		Query(const std::string &query);

		Query & operator<<(const std::string &s);
		Query &operator<<(int n);
		Query &operator<<(unsigned int n) { return operator<<((int)n); }
		Query &operator<<(double f);

		void Dump();

		std::string GetQuery() const { return query; }
		st_query_param GetParam(size_t n) const;
};

}

database::Query operator ""_sql(const char *str, size_t len);

#endif

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

#include <database/Query.hpp>
#include <excpt/Database.hpp>

#include <cmath>

using namespace std;

namespace database
{

Query::Query(const string &query)
{
	this->query = query;
}

Query & Query::operator<<(const std::string &s)
{
	st_query_param param;
	param.type = STRING;
	param.val_string = s;
	params.push_back(param);

	return *this;
}

Query & Query::operator<<(int n)
{
	st_query_param param;
	param.type = INT;
	param.val_int = n;
	params.push_back(param);

	return *this;
}

Query & Query::operator<<(double f)
{
	st_query_param param;

	if(isnan(f))
	{
		param.type = NULLVAL;
	}
	else
	{
		param.type = FLOAT;
		param.val_float = f;
	}

	params.push_back(param);

	return *this;
}

Query::st_query_param Query::GetParam(size_t n) const
{
	if(n>=params.size())
		throw excpt::Database("Out of range query parameter");

	return params[n];
}

}

database::Query operator ""_sql(const char *str, size_t len)
{
	return database::Query(std::string(str, len));
}

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

#include <database/Result.hpp>

#include <stdexcept>

using namespace std;

namespace database
{

Result::Result(MYSQL_RES *res)
{
	this->res = res;

	cols = mysql_num_fields(res);
	for(int i=0; i<cols; i++)
		col_name_idx.insert(std::pair<string, int>(string(mysql_fetch_field(res)->name), i));
}

Result::~Result()
{
	mysql_free_result(res);
}

bool Result::FetchRow(void)
{
	if(res==0)
		return false;

	row=mysql_fetch_row(res);
	row_field_length=mysql_fetch_lengths(res);

	if(row)
		return true;

	return false;
}

string Result::GetField(unsigned int n) const
{
	if(res==0 || row==0)
		throw out_of_range("Result is empty");

	if(n>=cols)
		throw out_of_range("Field number " + to_string(n) + " is out of range (only have + " + to_string(cols) + " columns)");

	return string(row[n], row_field_length[n]);
}

const ResultField Result::operator[](unsigned int n)
{
	return ResultField(GetField(n));
}
const ResultField Result::operator[](const std::string &name)
{
	auto col = col_name_idx.find(name);
	if(col==col_name_idx.end())
		throw out_of_range("Unknown column « " + name + " »");

	return ResultField(GetField(col->second));
}

}

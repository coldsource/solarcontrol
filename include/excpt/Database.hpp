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

#ifndef __EXCPT_DATABASE_HPP__
#define __EXCPT_DATABASE_HPP__

#include <excpt/Exception.hpp>

namespace excpt {

class Database: public Exception
{
	public:
		Database(const std::string &sql_err, unsigned int sql_errno):Exception("SQL error « " + sql_err + " » code " + std::to_string(sql_errno))
		{
			j_excpt["err"] = sql_err;
			j_excpt["errno"] = sql_errno;
		}

		Database(const std::string &what):Exception(what) {}

		virtual ~Database() {}
};

}

#endif


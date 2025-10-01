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

#include <excpt/Exception.hpp>
#include <logs/Logger.hpp>

namespace excpt {

using namespace std;
using nlohmann::json;

Exception *Exception::live_excpt = 0;

Exception::Exception(const string &what):runtime_error(what)
{
	live_excpt = this;

	j_excpt["message"] = string(what);
}

Exception::~Exception()
{
	live_excpt = 0;
}

void Exception::Log(int crit) const
{
	logs::Logger::Log(crit, string(what()));
	for(auto log : logs)
		logs::Logger::Log(LOG_NOTICE, log);
}

json Exception::ToJson() const
{
	return j_excpt;
}


}

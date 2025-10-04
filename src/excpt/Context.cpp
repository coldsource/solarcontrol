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

#include <excpt/Context.hpp>
#include <excpt/Exception.hpp>

namespace excpt {

using namespace std;
using nlohmann::json;

Context::Context(const string &name, const string &log, json j_ctx):
name(name),
log(log),
j_ctx(j_ctx),
excpt_live_before(Exception::live_excpt)
{}

Context::~Context()
{
	// Register context if 1. Live exception exists, 2. Context was created before exception
	if(Exception::live_excpt && !excpt_live_before)
	{
		j_ctx["log"] = log;
		Exception::live_excpt->j_excpt[name].push_back(j_ctx);
		Exception::live_excpt->logs.push_back(log);
	}
}

}


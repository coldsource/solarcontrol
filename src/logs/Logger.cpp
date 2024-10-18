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

#include <logs/Logger.hpp>

using namespace std;

namespace logs
{

void Logger::Log(int crit, const string &msg)
{
	if(crit==LOG_EMERG)
		fputs("EMERG ", stderr);
	else if(crit==LOG_ALERT)
		fputs("ALERT ", stderr);
	else if(crit==LOG_CRIT)
		fputs("CRIT ", stderr);
	else if(crit==LOG_ERR)
		fputs("ERR ", stderr);
	else if(crit==LOG_WARNING)
		fputs("WARNING ", stderr);
	else if(crit==LOG_NOTICE)
		fputs("NOTICE ", stderr);
	else if(crit==LOG_INFO)
		fputs("INFO ", stderr);
	else if(crit==LOG_DEBUG)
		fputs("DEBUG ", stderr);

	fputs(msg.c_str(), stderr);
	fputs("\n", stderr);
}

}

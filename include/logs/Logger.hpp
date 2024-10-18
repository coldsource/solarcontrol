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

#ifndef __LOGS_LOGGER_HPP__
#define __LOGS_LOGGER_HPP__

#define LOG_EMERG             0
#define LOG_ALERT             1
#define LOG_CRIT              2
#define LOG_ERR               3
#define LOG_WARNING           4
#define LOG_NOTICE            5
#define LOG_INFO              6
#define LOG_DEBUG             7

#include <string>

namespace logs
{

class Logger
{
	public:
		static void Log(int crit, const std::string &msg);
};

}

#endif

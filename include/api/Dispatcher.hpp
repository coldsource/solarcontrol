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

#ifndef __API_DISPATCHER_HPP__
#define __API_DISPATCHER_HPP__

#include <api/Handler.hpp>

#include <map>
#include <string>

namespace api {

class Dispatcher
{
	std::map<std::string, Handler *> handlers;

	public:
		Dispatcher();
		~Dispatcher();

		std::string Dispatch(const std::string &message);
};

}

#endif

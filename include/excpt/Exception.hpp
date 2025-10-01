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

#ifndef __EXCPT_EXCEPTION_HPP__
#define __EXCPT_EXCEPTION_HPP__

#include <excpt/Context.hpp>
#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>

namespace excpt {

class Exception: public std::runtime_error
{
	friend class Context;

	private:
		static Exception *live_excpt;

	protected:
		nlohmann::json j_excpt;
		std::vector<std::string> logs;

	public:
		Exception(const std::string &what);
		virtual ~Exception();

		virtual void Log(int crit) const final;
		virtual nlohmann::json ToJson() const final;
};

}

#endif

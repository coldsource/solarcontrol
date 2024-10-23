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

#ifndef __ENERGY_HISTORY_HPP__
#define __ENERGY_HISTORY_HPP__

#include <datetime/Date.hpp>

#include <string>
#include <map>

namespace energy {

class History
{
	std::string type;
	std::map<datetime::Date, double> history;

	void purge(int ndays);

	public:
		History(const std::string &type = "");
		~History();

		void Set(double energy);
		void Add(double energy);
		void Save();

		double GetTotalForLast(int ndays) const;
		const std::map<datetime::Date, double> &Get() const { return history; }
};

}

#endif


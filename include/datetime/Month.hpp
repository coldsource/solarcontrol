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

#ifndef __DATETIME_MONTH_HPP__
#define __DATETIME_MONTH_HPP__

#include <datetime/Date.hpp>

#include <string>

namespace datetime {

class Timestamp;

class Month: public Date
{
	public:
		Month();
		Month(const Timestamp &ts);
		Month(const std::string &str);

		Month operator-(int months) const;
		Month operator+(int months) const;
};

}

#endif

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

#include <datetime/Month.hpp>
#include <datetime/Timestamp.hpp>

#include <stdexcept>

using namespace std;

namespace datetime {

Month::Month(): Date()
{
	ToMonth();
}

Month::Month(const Timestamp &ts): Date(ts)
{
	ToMonth();
}

Month::Month(const string &str): Date(str)
{
	ToMonth();
}

Month Month::operator-(int months) const
{
	Month before_m(*this);

	before_m.tm.tm_mon -= months;
	mktime(&before_m.tm);

	return before_m;
}

Month Month::operator+(int months) const
{
	Month after_m(*this);

	after_m.tm.tm_mon += months;
	mktime(&after_m.tm);

	return after_m;
}

}

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

#include <datetime/HourMinuteSecond.hpp>

#include <regex>
#include <stdexcept>

using namespace std;

namespace datetime {

HourMinuteSecond::HourMinuteSecond(const string &s)
{
	regex hms("^([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})$");
	smatch matches;

	if(!regex_search(s, matches, hms))
		throw invalid_argument("Invalid hour:minute:second format : « " + s + " »");

	hour = stoi(matches[1]);
	minute = stoi(matches[2]);
	second = stoi(matches[3]);
}

HourMinuteSecond::HourMinuteSecond()
{
	hour = 0;
	minute = 0;
	second = 0;
}

}

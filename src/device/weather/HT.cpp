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

#include <device/weather/HT.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>

#include <limits>

using nlohmann::json;
using namespace std;

namespace device {

HT::HT(int id):
Weather(id), history(id)
{
}

HT::~HT()
{
}

void HT::state_restore(const  configuration::Json &last_state)
{
	humidity = last_state.GetFloat("humidity", std::numeric_limits<double>::quiet_NaN());
	temperature = last_state.GetFloat("temperature", std::numeric_limits<double>::quiet_NaN());

	Device::state_restore(last_state);
}

configuration::Json HT::state_backup()
{
	auto backup = Device::state_backup();

	if(!std::isnan(GetHumidity()))
		backup.Set("humidity", GetHumidity());

	if(!std::isnan(GetTemperature()))
		backup.Set("temperature", GetTemperature());

	return backup;
}

}

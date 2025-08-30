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

#include <device/weather/DeviceHT.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>

#include <limits>

using nlohmann::json;
using namespace std;

namespace device {

DeviceHT::DeviceHT(int id):
DeviceWeather(id), history(id)
{
	auto state = state_restore();

	humidity = state.GetFloat("humidity", std::numeric_limits<double>::quiet_NaN());
	temperature = state.GetFloat("temperature", std::numeric_limits<double>::quiet_NaN());
}

DeviceHT::~DeviceHT()
{
	json state;
	if(!std::isnan(GetHumidity()))
		state["humidity"] = GetHumidity();

	if(!std::isnan(GetTemperature()))
		state["temperature"] = GetTemperature();

	state_backup(configuration::Json(state));
}

void DeviceHT::Log()
{
	double h = GetHumidity();
	double t = GetTemperature();

	if(std::isnan(h) || std::isnan(t))
		return;

	weather::MinMaxHT ht(h, t);
	history.Add(ht);
}

}

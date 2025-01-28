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

#include <device/DeviceHT.hpp>
#include <control/HT.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>

#include <limits>

using nlohmann::json;

namespace device {

DeviceHT::DeviceHT(unsigned int id, const std::string &name, const configuration::Json &config, control::HT *ctrl):
DeviceWeather(id, name, config), ctrl(ctrl), history(id)
{
	auto state = state_restore();

	double h = state.GetFloat("humidity", std::numeric_limits<double>::quiet_NaN());
	double t = state.GetFloat("temperature", std::numeric_limits<double>::quiet_NaN());

	ctrl->SetHT(h, t);
}

DeviceHT::~DeviceHT()
{
	json state;
	if(!std::isnan(GetHumidity()))
		state["humidity"] = GetHumidity();

	if(!std::isnan(GetTemperature()))
		state["temperature"] = GetTemperature();

	state_backup(configuration::Json(state));

	delete ctrl;
}

double DeviceHT::GetTemperature() const
{
	return ctrl->GetTemperature();
}

double DeviceHT::GetHumidity() const
{
	return ctrl->GetHumidity();
}

double DeviceHT::GetWind() const
{
	return std::numeric_limits<double>::quiet_NaN();
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

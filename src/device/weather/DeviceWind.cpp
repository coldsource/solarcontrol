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


#include <device/weather/DeviceWind.hpp>
#include <control/Wind.hpp>
#include <configuration/Json.hpp>
#include <stat/MinMaxAvg.hpp>

using namespace std;

namespace device
{

DeviceWind::DeviceWind(unsigned int id, const string &name, const configuration::Json &config):
DeviceWeather(id, name, config), history(id)
{
	ctrl = new control::Wind(config.GetString("mqtt_id"));
}

DeviceWind::~DeviceWind()
{
	delete ctrl;
}

double DeviceWind::GetTemperature() const
{
	return std::numeric_limits<double>::quiet_NaN();
}

double DeviceWind::GetHumidity() const
{
	return std::numeric_limits<double>::quiet_NaN();
}

double DeviceWind::GetWind() const
{
	return ctrl->GetWind();
}

void DeviceWind::Log()
{
	double w = GetWind();

	if(std::isnan(w))
		return;

	history.Add(stat::MinMaxAvg(w));
}

}


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
#include <sensor/weather/Wind.hpp>
#include <configuration/Json.hpp>
#include <stat/MinMaxAvg.hpp>

using namespace std;

namespace device
{

DeviceWind::DeviceWind(int id):DeviceWeather(id), history(id)
{
}

DeviceWind::~DeviceWind()
{
}

void DeviceWind::CheckConfig(const configuration::Json &conf)
{
	DeviceWeather::CheckConfig(conf);

	sensor::weather::Wind::CheckConfig(conf);
}

void DeviceWind::reload(const configuration::Json &config)
{
	DeviceWeather::reload(config);

	add_sensor(make_shared<sensor::weather::Wind>(config.GetString("mqtt_id")), "ht");
}

void DeviceWind::Log()
{
	double w = GetWind();

	if(std::isnan(w))
		return;

	history.Add(stat::MinMaxAvg(w));
}

void DeviceWind::SensorChanged(const sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	wind = ((sensor::weather::Wind *)sensor)->GetWind();
}

}


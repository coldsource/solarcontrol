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


#include <device/weather/Wind.hpp>
#include <sensor/weather/Wind.hpp>
#include <configuration/Json.hpp>
#include <stat/MinMaxAvg.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

Wind::Wind(int id):Weather(id), history(id)
{
}

Wind::~Wind()
{
}

void Wind::state_restore(const  configuration::Json &last_state)
{
	wind = last_state.GetFloat("wind", std::numeric_limits<double>::quiet_NaN());

	Device::state_restore(last_state);
}

configuration::Json Wind::state_backup()
{
	auto backup = Device::state_backup();

	if(!std::isnan(GetWind()))
		backup.Set("wind", GetWind());

	return backup;
}

void Wind::CheckConfig(const configuration::Json &conf)
{
	Weather::CheckConfig(conf);

	sensor::weather::Wind::CheckConfig(conf);
}

void Wind::reload(const configuration::Json &config)
{
	Weather::reload(config);

	add_sensor(make_shared<sensor::weather::Wind>(config.GetString("mqtt_id")), "ht");
}

void Wind::SensorChanged(const sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	wind = ((sensor::weather::Wind *)sensor)->GetWind();

	history.Add(stat::MinMaxAvg(wind));
}

}


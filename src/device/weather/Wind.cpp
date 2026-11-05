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
#include <sensor/weather/FactoryWind.hpp>
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

shared_ptr<Device::Configuration> Wind::build_config(const configuration::Json &j_config) const
{
	return make_shared<Wind::Configuration>(j_config);
}

Wind::Configuration::Configuration(const configuration::Json &config):Weather::Configuration(config)
{
	add_sensor<sensor::weather::FactoryWind>("ht", config);
}

void Wind::sensor_changed(const sensor::Sensor *sensor)
{
	wind = ((sensor::weather::Wind *)sensor)->GetWind();

	history.Add(stat::MinMaxAvg((double)wind));
}

void Wind::SyncHistory()
{
	lock_guard<recursive_mutex> llock(lock);
	history.Sync();
}

}


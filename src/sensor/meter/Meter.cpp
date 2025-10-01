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

#include <sensor/meter/Meter.hpp>
#include <configuration/Json.hpp>
#include <excpt/Config.hpp>

using namespace std;

namespace sensor::meter {

void Meter::CheckConfig(const configuration::Json &conf)
{
	conf.Check("mqtt_id", "string");

	if(conf.GetString("mqtt_id")=="")
		throw excpt::Config("Missing MQTT ID", "mqtt_id");
}

double Meter::GetPower() const
{
	return power;
}

double Meter::GetConsumption()
{
	unique_lock<mutex> llock(lock);

	double ret_consumption = energy_consumption;
	energy_consumption = 0;

	return ret_consumption;
}

double Meter::GetExcess()
{
	unique_lock<mutex> llock(lock);

	double ret_excess = energy_excess;
	energy_excess = 0;

	return ret_excess;
}

}

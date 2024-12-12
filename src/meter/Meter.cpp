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

#include <meter/Meter.hpp>
#include <meter/Dummy.hpp>
#include <meter/Plug.hpp>
#include <meter/Pro3EM.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;

namespace meter {

Meter *Meter::GetFromConfig(const configuration::Json &conf)
{
	string type = conf.GetString("type");
	if(type=="plug")
		return new Plug(conf.GetString("mqtt_id"));
	if(type=="pro")
		return new Dummy(); // Shelly Pro has no energy measurement
	if(type=="3em")
		return new Pro3EM(conf.GetString("mqtt_id"), conf.GetString("phase"));

	throw invalid_argument("Unknown meter type « " + type + " »");
}

double Meter::GetPower() const
{
	unique_lock<mutex> llock(lock);

	return power;
}

}

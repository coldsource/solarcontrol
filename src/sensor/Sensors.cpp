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

#include <sensor/Sensors.hpp>
#include <sensor/Sensor.hpp>

using namespace std;

namespace sensor {

Sensors::~Sensors()
{
	clear();
}

void Sensors::insert(const std::string &name, SensorObserver *observer, std::shared_ptr<Sensor> sensor)
{
	sensor->SetName(name);
	sensor->SetObserver(observer);

	sensors.insert(sensor);
}

void Sensors::clear()
{
	for(auto sensor : sensors)
		sensor->SetObserver();
	sensors.clear();

}

}

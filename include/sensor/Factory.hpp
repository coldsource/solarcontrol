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

#ifndef __SENSOR_FACTORY_HPP__
#define __SENSOR_FACTORY_HPP__

#include <string>
#include <memory>

namespace configuration {
	class Json;
}

namespace sensor {

class SensorObserver;
class Sensor;

class Factory
{
	std::string name = "";

	SensorObserver *observer = 0;

	protected:
		virtual std::shared_ptr<Sensor> get_from_config(const std::string &name, SensorObserver *observer, const configuration::Json &conf) = 0;

	public:
		Factory(SensorObserver *observer):observer(observer) {}
		virtual ~Factory() {}

		virtual std::shared_ptr<Sensor> GetFromConfig(const std::string &name, const configuration::Json &conf) final
		{
			return get_from_config(name, observer, conf);
		}
};

}

#endif


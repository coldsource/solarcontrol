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

#ifndef __CONTROL_HT_HPP__
#define __CONTROL_HT_HPP__

#include <mqtt/Subscriber.hpp>

#include <string>
#include <mutex>

namespace control {

class HT: public mqtt::Subscriber
{
	double temperature;
	double humidity;

	mutable std::mutex lock;

	public:
		HT(const std::string &mqtt_id);
		virtual ~HT() {}

		double GetTemperature() const;
		double GetHumidity() const;

		void HandleMessage(const std::string &message);
};

}

#endif



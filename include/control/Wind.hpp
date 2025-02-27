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

#ifndef __CONTROL_WIND_HPP__
#define __CONTROL_WIND_HPP__

#include <mqtt/Subscriber.hpp>

#include <mutex>
#include <atomic>

namespace control {

class Wind: public mqtt::Subscriber
{
	const std::string topic;

	std::atomic<double> wind = std::numeric_limits<double>::quiet_NaN();

	std::mutex lock;

	public:
		Wind(const std::string &mqtt_id);
		virtual ~Wind();

		double GetWind() const;
		void SetWind(double w);

		void HandleMessage(const std::string &message);
};

}

#endif




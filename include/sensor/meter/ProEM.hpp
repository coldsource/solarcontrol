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

#ifndef __SENSOR_METER_EM_HPP__
#define __SENSOR_METER_EM_HPP__

#include <sensor/meter/Meter.hpp>
#include <mqtt/Subscriber.hpp>

namespace configuration {
	class Json;
}

namespace sensor::meter {

class ProEM: public Meter, public mqtt::Subscriber
{
	protected:
		std::string topic;

		int phasei;
		double power_phases[2] = {0, 0};

		double last_energy_consumption_phases[2] = {0, 0};
		double last_energy_excess_phases[2] = {0, 0};

	public:
		ProEM(const std::string &mqtt_id, const std::string &phase);
		virtual ~ProEM();

		static void CheckConfig(const configuration::Json &conf);

		void HandleMessage(const std::string &message, const std::string & /*topic*/) override;
};

}

#endif

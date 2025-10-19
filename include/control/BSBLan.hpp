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

#ifndef __CONTROL_BSBLAN_HPP__
#define __CONTROL_BSBLAN_HPP__

#include <control/OnOff.hpp>
#include <http/HTTP.hpp>

#include <nlohmann/json.hpp>

#include <string>
#include <mutex>

namespace configuration {
	class Json;
}

namespace control {

class BSBLan: public OnOff
{
	http::HTTP http;

	std::mutex lock;

	void set(int param, int value) const;
	void set(int param, double value) const;
	void send(const nlohmann::json &params) const;

	public:
		BSBLan(const std::string &ip):http(ip) {}
		virtual ~BSBLan() {}

		static void CheckConfig(const configuration::Json & conf);

		void Switch(bool state) override;
		void SetAmbientTemperature(double temp) const;
		void SetEcoSetPoint(double temp) const;
		void SetComfortSetPoint(double temp) const;
};

}

#endif



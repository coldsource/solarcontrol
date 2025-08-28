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

#ifndef __DEVICE_DEVICEWIND_HPP__
#define __DEVICE_DEVICEWIND_HPP__

#include <device/weather/DeviceWeather.hpp>
#include <weather/HistoryQuarterHourWind.hpp>

#include <string>
#include <memory>

namespace control {
	class Wind;
}

namespace device {

class DeviceWind: public DeviceWeather
{
	protected:
		std::shared_ptr<control::Wind> ctrl;

		weather::HistoryQuarterHourWind history;

	public:
		DeviceWind(int id);
		virtual ~DeviceWind();

		static void CheckConfig(const configuration::Json &conf);
		virtual void Reload(const std::string &name, const configuration::Json &config) override;

		std::string GetType() const override { return "wind"; }

		double GetTemperature() const override;
		double GetHumidity() const override;
		double GetWind() const override;

		void Log() override;
};

}

#endif




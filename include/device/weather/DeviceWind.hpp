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
#include <limits>

namespace control {
	class Wind;
}

namespace device {

class DeviceWind: public DeviceWeather
{
	double wind;

	weather::HistoryQuarterHourWind history;

	protected:
		virtual void reload(const configuration::Json &config) override;

	public:
		DeviceWind(int id);
		virtual ~DeviceWind();

		static void CheckConfig(const configuration::Json &conf);

		std::string GetType() const override { return "wind"; }

		double GetTemperature() const override { return std::numeric_limits<double>::quiet_NaN(); }
		double GetHumidity() const override { return std::numeric_limits<double>::quiet_NaN(); }
		double GetWind() const override { return wind; }

		virtual void SensorChanged(const sensor::Sensor *sensor) override;
};

}

#endif




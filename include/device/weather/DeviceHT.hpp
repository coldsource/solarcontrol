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

#ifndef __DEVICE_DEVICEHT_HPP__
#define __DEVICE_DEVICEHT_HPP__

#include <device/weather/DeviceWeather.hpp>
#include <weather/HistoryQuarterHourHT.hpp>
#include <configuration/Json.hpp>

#include <memory>
#include <limits>

namespace control {
	class HT;
}

namespace device {

class DeviceHT: public DeviceWeather
{
	protected:
		double humidity;
		double temperature;

		weather::HistoryQuarterHourHT history;

		virtual void state_restore(const  configuration::Json &last_state) override;
		virtual configuration::Json state_backup() override;

	public:
		DeviceHT(int id);
		virtual ~DeviceHT();

		double GetTemperature() const override { return temperature; }
		double GetHumidity() const override { return humidity; }
		double GetWind() const override { return std::numeric_limits<double>::quiet_NaN(); }
};

}

#endif




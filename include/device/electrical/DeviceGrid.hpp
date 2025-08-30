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

#ifndef __DEVICE_DEVICEGRID_HPP__
#define __DEVICE_DEVICEGRID_HPP__

#include <device/electrical/DevicePassive.hpp>
#include <sensor/SensorObserver.hpp>

#include <memory>

namespace configuration {
	class Json;
}

namespace device {

class DeviceGrid: public DevicePassive
{
	bool offpeak = false;

	protected:
		virtual void reload(const configuration::Json &config) override;

	public:
		DeviceGrid(int id);
		virtual ~DeviceGrid();

		static void CheckConfig(const configuration::Json &conf);

		std::string GetType() const override { return "grid"; }

		const std::map<datetime::Date, energy::Amount> &GetExcessHistory() const { return consumption.GetExcessHistory(); }

		bool GetOffPeak() const { return offpeak; }

		virtual void SensorChanged(const sensor::Sensor *sensor) override;

		static void CreateInDB();
};

}

#endif

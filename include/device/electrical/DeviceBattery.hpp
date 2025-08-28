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

#ifndef __DEVICE_DEVICEBATTERY_HPP__
#define __DEVICE_DEVICEBATTERY_HPP__

#include <device/electrical/DevicePassive.hpp>

#include <memory>

namespace configuration {
	class Json;
}

namespace meter {
	class Voltmeter;
}

namespace device {

class DeviceBattery: public DevicePassive
{
	protected:
		std::unique_ptr<meter::Voltmeter> voltmeter;

	public:
		DeviceBattery(int id);
		virtual ~DeviceBattery();

		std::string GetType() const override { return "battery"; }

		const std::map<datetime::Date, energy::Amount> &GetProductionHistory() const { return consumption.GetConsumptionHistory(); }

		static void CheckConfig(const configuration::Json &conf);
		virtual void Reload(const std::string &name, const configuration::Json &config) override;

		double GetVoltage() const;
		double GetSOC() const;
		virtual nlohmann::json ToJson() const override;

		static void CreateInDB();
};

}

#endif


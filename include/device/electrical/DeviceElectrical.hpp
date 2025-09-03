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

#ifndef __DEVICE_DEVICEELECTRICAL_HPP__
#define __DEVICE_DEVICEELECTRICAL_HPP__

#include <device/Device.hpp>
#include <energy/Counter.hpp>

#include <string>
#include <memory>

namespace control {
	class OnOff;
}

namespace device {

class DeviceElectrical: public Device
{
	protected:
		double power = -1;

		energy::Counter consumption;
		energy::Counter offload;

		virtual void reload(const configuration::Json &config) override;

	public:
		DeviceElectrical(int id);
		virtual ~DeviceElectrical();

		static void CheckConfig(const configuration::Json &conf);

		double GetPower() const { return power; }

		energy::Amount GetEnergyConsumption() { return consumption.GetEnergyConsumption(); }
		energy::Amount GetEnergyOffload() { return offload.GetEnergyConsumption(); }
		energy::Amount GetEnergyExcess() { return consumption.GetEnergyExcess(); }

		const std::map<datetime::Date, energy::Amount> &GetConsumptionHistory() const { return consumption.GetConsumptionHistory(); }
		const std::map<datetime::Date, energy::Amount> &GetOffloadHistory() const { return offload.GetConsumptionHistory(); }

		virtual nlohmann::json ToJson() const override;

		virtual void SensorChanged(const sensor::Sensor *sensor) override;
};

}

#endif

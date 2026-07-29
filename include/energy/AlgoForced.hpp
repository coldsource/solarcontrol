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

#ifndef __ENERGY_ALGOFORCED_HPP__
#define __ENERGY_ALGOFORCED_HPP__

#include <energy/DeviceOnOffAlgo.hpp>

#include <memory>
#include <map>

namespace device{
	class OnOff;
}

namespace energy {

class AlgoForced: public DeviceOnOffAlgo
{
	protected:
		const std::map<std::shared_ptr<device::OnOff>, bool> devices;
		double forced_power, forced_power_grid, forced_power_battery;
		bool state_changed;

	public:
		AlgoForced(const std::map<std::shared_ptr<device::OnOff>, bool> &devices):devices(devices) {}

		virtual void Run() override;
		virtual double GetEnabledPower() const override { return forced_power; };
		double GetEnabledPowerGrid() const { return forced_power_grid; }
		double GetEnabledPowerBattery() const { return forced_power_battery; }
		virtual bool HasStateChanged() const override { return state_changed; }
};

}

#endif




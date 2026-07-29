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

#ifndef __ENERGY_ALGOOFFLOADBATTERY_HPP__
#define __ENERGY_ALGOOFFLOADBATTERY_HPP__

#include <energy/DeviceOnOffAlgo.hpp>

#include <memory>
#include <vector>

namespace device{
	class OnOff;
}

namespace thread {
	class Stats;
}

namespace energy {

class AlgoOffloadBattery: public DeviceOnOffAlgo
{
	protected:
		// State
		const std::vector<std::shared_ptr<device::OnOff>> devices;
		double offloaded_power;
		bool state_changed;

		thread::Stats *stats = 0;

	public:
		AlgoOffloadBattery(const std::vector<std::shared_ptr<device::OnOff>> &devices):devices(devices) {}

		virtual void Run() override;
		virtual double GetEnabledPower() const override { return offloaded_power; };
		virtual bool HasStateChanged() const override { return state_changed; }
};

}

#endif






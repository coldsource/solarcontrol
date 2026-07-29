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

#ifndef __ENERGY_ALGOOFFLOAD_HPP__
#define __ENERGY_ALGOOFFLOAD_HPP__

#include <energy/DeviceOnOffAlgo.hpp>
#include <configuration/ConfigurationObserver.hpp>

#include <memory>
#include <vector>

namespace device{
	class OnOff;
}

namespace thread {
	class Stats;
}

namespace energy {

class AlgoOffload: public DeviceOnOffAlgo, public configuration::ConfigurationObserver
{
	protected:
		// Config
		int hysteresis_export = 0;
		int hysteresis_import = 0;
		double hysteresis_precision = 0;

		// State
		const std::vector<std::shared_ptr<device::OnOff>> devices;
		double forced_power;
		double offloaded_power;
		bool state_changed;

		thread::Stats *stats = 0;

		bool hysteresis(const std::shared_ptr<device::OnOff> device) const;

	public:
		AlgoOffload(const std::vector<std::shared_ptr<device::OnOff>> &devices, double forced_power);

		void ConfigurationChanged(const configuration::ConfigurationPart *config) override;

		virtual void Run() override;
		virtual double GetEnabledPower() const override { return offloaded_power; };
		virtual bool HasStateChanged() const override { return state_changed; }
};

}

#endif





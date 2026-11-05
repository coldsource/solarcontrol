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

#ifndef __DEVICE_PV_HPP__
#define __DEVICE_PV_HPP__

#include <device/electrical/Passive.hpp>

namespace configuration {
	class Json;
}

namespace device {

class PV: public Passive
{
	protected:
		class Configuration: public Passive::Configuration
		{
			public:
				Configuration(const configuration::Json &config): Passive::Configuration(config) {}
		};

		virtual std::shared_ptr<Device::Configuration> build_config(const configuration::Json &j_config) const override;
		inline std::shared_ptr<const Configuration> get_config() const { return dynamic_pointer_cast<const PV::Configuration>(config.load()); }

	public:
		PV(int id);
		virtual ~PV() {}

		std::string GetType() const override { return "pv"; }

		const std::map<datetime::Date, energy::Amount> GetProductionHistory() const { return GetConsumptionHistory(); }

		static void CreateInDB();
};

}

#endif

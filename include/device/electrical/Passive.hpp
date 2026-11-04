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

#ifndef __DEVICE_PASSIVE_HPP__
#define __DEVICE_PASSIVE_HPP__

#include <device/electrical/Electrical.hpp>

#include <string>

namespace device {

class Passive: public Electrical
{
	protected:
		class Configuration: public Electrical::Configuration
		{
			public:
				Configuration(const configuration::Json &config):Electrical::Configuration(config) {}
		};

		virtual std::shared_ptr<Device::Configuration> build_config(const configuration::Json &j_config) const override;
		inline std::shared_ptr<const Configuration> get_config() const { return dynamic_pointer_cast<const Passive::Configuration>(config.load()); }

public:
		Passive(int id): Electrical(id) {}
		Passive(int id, const std::string &consumption_type, const std::string consumption_excess_type): Electrical(id, consumption_type, consumption_excess_type) {}

		static void CheckConfig(const configuration::Json &conf);

		en_category GetCategory() const override { return PASSIVE; }
		std::string GetType() const override { return "passive"; }
};

}

#endif



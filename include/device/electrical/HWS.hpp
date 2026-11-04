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

#ifndef __DEVICE_HWS_HPP__
#define __DEVICE_HWS_HPP__

#include <device/electrical/TimeRange.hpp>

namespace configuration {
	class Json;
}

namespace device {

class HWS: public TimeRange
{
	protected:
		// Device configuration
		class Configuration: public TimeRange::Configuration
		{
			public:
				double min_energy; // kWh
				int min_energy_for_last; // Days

				Configuration(const configuration::Json &config);
		};

		virtual std::shared_ptr<Device::Configuration> build_config(const configuration::Json &j_config) const override;
		inline std::shared_ptr<const Configuration> get_config() const { return dynamic_pointer_cast<const HWS::Configuration>(config.load()); }

		// Global configuration
		std::atomic_bool absence;

	public:
		HWS(int id);
		virtual ~HWS();

		void ConfigurationChanged(const configuration::ConfigurationPart * config) override;

		static void CheckConfig(const configuration::Json &conf);

		bool WantRemainder(configuration::Json *data_ptr = 0) const override;

		std::string GetType() const override { return "hws"; }

		static void CreateInDB();
};

}

#endif




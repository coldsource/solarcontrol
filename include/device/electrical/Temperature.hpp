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

#ifndef __DEVICE_TEMPERATURE_HPP__
#define __DEVICE_TEMPERATURE_HPP__

#include <device/electrical/TimeRange.hpp>

namespace configuration {
	class Json;
}

namespace device {

class Temperature: public TimeRange
{
	private:
		// Global config
		struct st_absence
		{
			bool enabled;
			double temperature;
		};

		std::atomic<std::shared_ptr<st_absence>> absence;

	protected:
		class Configuration: public TimeRange::Configuration
		{
			public:
				int ht_device_id;

				Configuration(const configuration::Json &config);
		};

		static void check_timeranges(const configuration::Json &conf, const std::string &name);

		virtual en_wanted_state get_wanted_state(configuration::Json *data_ptr = 0) const override;
		virtual bool temp_check_force(double current_temp, double timerange_temp) const = 0;
		virtual bool temp_check_offload(double current_temp, double timerange_temp) const = 0;

		virtual std::shared_ptr<Device::Configuration> build_config(const configuration::Json &j_config) const override;
		inline std::shared_ptr<const Configuration> get_config() const { return dynamic_pointer_cast<const Temperature::Configuration>(config.load()); }
		inline std::shared_ptr<st_absence> get_absence() const { return absence.load(); }

	public:
		Temperature(int id);
		virtual ~Temperature();

		void ConfigurationChanged(const configuration::ConfigurationPart * config) override;

		static void CheckConfig(const configuration::Json &conf);

		bool Depends(int device_id) const override;

		virtual nlohmann::json ToJson() const override;
};

}

#endif



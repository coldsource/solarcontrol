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

#ifndef __DEVICE_GRID_HPP__
#define __DEVICE_GRID_HPP__

#include <device/electrical/Passive.hpp>
#include <sensor/SensorObserver.hpp>
#include <nlohmann/json.hpp>

#include <memory>

namespace configuration {
	class Json;
}

namespace device {

class Grid: public Passive
{
	public:
		enum en_grid_state {UNKNOWN, ONLINE, OFFLINE};

	protected:
		virtual void reload(const configuration::Json &config) override;

		static en_grid_state string_to_grid_state(const std::string &str);
		static std::string grid_state_to_string(en_grid_state state);

		// State
		std::atomic_bool offpeak = false;
		std::atomic<en_grid_state> grid_state = UNKNOWN;

	public:
		Grid(int id);
		virtual ~Grid();

		static void CheckConfig(const configuration::Json &conf);

		std::string GetType() const override { return "grid"; }

		const std::map<datetime::Date, energy::Amount> &GetExcessHistory() const { return consumption.GetExcessHistory(); }

		bool GetOffPeak() const { return offpeak; }
		en_grid_state GetState() const { return grid_state; }

		virtual void SensorChanged(const sensor::Sensor *sensor) override;

		virtual nlohmann::json ToJson() const override;

		static void CreateInDB();
};

}

#endif

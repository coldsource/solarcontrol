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

#ifndef __DEVICE_DEVICE_HPP__
#define __DEVICE_DEVICE_HPP__

#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <mutex>

namespace device {

enum en_category
{
	ANY,
	ONOFF,
	PASSIVE,
	WEATHER
};

class Device
{
	int id;
	configuration::Json config;
	std::string name;

	bool deleted = false;

	protected:
		std::recursive_mutex mutex;

		void state_backup(const configuration::Json &state);
		const configuration::Json state_restore();

	public:
		Device(int id):id(id) {}
		Device(const Device&) = delete;
		virtual ~Device();

		static void CheckConfig(const configuration::Json & /* conf */) {}
		virtual void Reload(const std::string &name, const configuration::Json &config);

		virtual std::string GetType() const = 0;
		virtual en_category GetCategory() const = 0;
		virtual nlohmann::json ToJson() const = 0;

		int GetID() const { return id; }
		std::string GetName() const { return name; }
		const configuration::Json GetConfig() const { return config; }

		virtual bool Depends(int /* device_id */) const { return false; }

		void Delete() { deleted = true; } // Device might be in use, flag for removal in destructor
};

}

#endif

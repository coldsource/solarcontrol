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

#include <sensor/Sensors.hpp>
#include <sensor/SensorObserver.hpp>
#include <configuration/Json.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <mutex>
#include <set>
#include <memory>

namespace sensor {
	class Sensor;
}

namespace device {

enum en_category
{
	ANY,
	ONOFF,
	PASSIVE,
	WEATHER
};

class Device: public sensor::SensorObserver
{
	int id;
	configuration::Json config;
	std::string name;

	sensor::Sensors sensors;

	bool deleted = false;

	protected:
		mutable std::recursive_mutex lock;

		bool state_restored = false;

		void add_sensor(std::shared_ptr<sensor::Sensor> sensor, const std::string &name);

		virtual void reload(const configuration::Json & /* config */) {}
		virtual void state_restore(const configuration::Json & /* last_state */ ) {}
		virtual configuration::Json state_backup() { return configuration::Json(); }

	public:
		Device(int id):id(id) {}
		Device(const Device&) = delete;
		virtual ~Device();

		static void CheckConfig(const configuration::Json & /* conf */) {}
		virtual void Reload(const std::string &name, const configuration::Json &config) final;
		void Delete();

		virtual void StateRestore(const configuration::Json &last_state) final;
		virtual  configuration::Json StateBackup() { return state_backup(); }

		virtual std::string GetType() const = 0;
		virtual en_category GetCategory() const = 0;
		virtual nlohmann::json ToJson() const = 0;

		int GetID() const { return id; }
		std::string GetName() const;
		const configuration::Json GetConfig() const;

		virtual bool Depends(int /* device_id */) const { return false; }

		static bool CompareTo(std::shared_ptr<Device> a, std::shared_ptr<Device> b) // By default devices are sorted by name
		{
			return a->GetName() < b->GetName();
		}
};

}

#endif

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

#ifndef __SHELLY_AUTODETECTBLE_HPP__
#define __SHELLY_AUTODETECTBLE_HPP__

#include <mqtt/Subscriber.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <mutex>
#include <condition_variable>

namespace shelly {

class AutodetectBLE: public mqtt::Subscriber
{
	std::mutex wait_lock;
	std::condition_variable ble_detected;

	nlohmann::json j_res;

	public:
		AutodetectBLE();
		~AutodetectBLE();

		void HandleMessage(const std::string & /*message*/, const std::string &topic) override;

		nlohmann::json GetDevice();
};

}

#endif




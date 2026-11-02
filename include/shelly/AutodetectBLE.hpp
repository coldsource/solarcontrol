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
#include <mqtt/Dispatcher.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace shelly {

class AutodetectBLESubscriber: public mqtt::Subscriber
{
	protected:
		std::shared_ptr<mqtt::Dispatcher> dispatcher;

	public:
		AutodetectBLESubscriber(std::shared_ptr<mqtt::Dispatcher> dispatcher):dispatcher(dispatcher) {}

		virtual std::shared_ptr<mqtt::Dispatcher> Get() const override
		{
			return dispatcher;
		}
};

class AutodetectBLE: public mqtt::Dispatcher, public std::enable_shared_from_this<AutodetectBLE>
{
	std::mutex wait_lock;
	std::condition_variable ble_detected;
	std::shared_ptr<AutodetectBLESubscriber> mqtt_subscriber = nullptr;

	nlohmann::json j_res;

	public:
		AutodetectBLE();
		~AutodetectBLE();

		virtual void DispatchMQTTMessage(const mqtt::st_message &mqtt_message) override;

		nlohmann::json GetDevice();
};

}

#endif

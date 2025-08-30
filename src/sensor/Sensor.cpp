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

#include <sensor/Sensor.hpp>
#include <sensor/SensorObserver.hpp>
#include <thread/SensorsManager.hpp>
#include <websocket/SolarControl.hpp>

using namespace std;

namespace sensor {

void Sensor::SetObserver(SensorObserver * observer)
{
	unique_lock<mutex> llock(observer_lock);

	this->observer = observer;

	// Register sensor for management
	::thread::SensorsManager::GetInstance()->Register(shared_from_this());
}

void Sensor::SetObserver()
{
	unique_lock<mutex> llock(observer_lock);

	this->observer = 0;

	// We have no more observers, unregister for future removal
	::thread::SensorsManager::GetInstance()->Unregister(shared_from_this());
}

void Sensor::notify_observer()
{
	{
		unique_lock<mutex> llock(observer_lock);

		if(observer==0)
			return; // No observer

		observer->SensorChanged(this);
	}

	// Signal changes to Websockets
	websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::DEVICE);
	websocket::SolarControl::GetInstance()->NotifyAll(websocket::SolarControl::en_protocols::METER);
}

}

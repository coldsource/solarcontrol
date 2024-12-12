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

#include <device/Devices.hpp>
#include <websocket/SolarControl.hpp>

namespace device
{

Devices *Devices::instance = 0;

Devices::Devices()
{
	instance = this;
}

void Devices::Reload()
{
	devices_onoff.d_mutex.lock();
	devices_onoff.reload();
	devices_onoff.d_mutex.unlock();

	devices_ht.d_mutex.lock();
	devices_ht.reload();
	devices_ht.d_mutex.unlock();

	devices_passive.d_mutex.lock();
	devices_passive.reload();
	devices_passive.d_mutex.unlock();

}

void Devices::Unload()
{
	devices_onoff.unload();
	devices_ht.unload();
	devices_passive.unload();
}

}

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

#ifndef __DEVICE_DEVICES_HPP__
#define __DEVICE_DEVICES_HPP__

#include <device/DevicesOnOff.hpp>
#include <device/DevicesHT.hpp>

#include <string>

namespace device {

class Devices
{
	static Devices *instance;

	DevicesOnOff devices_onoff;
	DevicesHT devices_ht;

	public:
		Devices();

		static Devices *GetInstance() { return instance; }

		void Reload();

		DevicesOnOff &GetOnOff() { return devices_onoff; }
		DevicesHT &GetHT() { return devices_ht; }
};

}

#endif


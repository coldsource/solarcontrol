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

#include <device/DevicesOnOffImpl.hpp>
#include <device/DevicesHTImpl.hpp>
#include <device/DevicesPassiveImpl.hpp>

#include <string>

#define DEVICE_ID_GRID     -1
#define DEVICE_ID_PV       -2
#define DEVICE_ID_HWS      -3

#define DEVICE_NAME_GRID    "grid"
#define DEVICE_NAME_PV      "pv"
#define DEVICE_NAME_HWS     "hws"

namespace device {

class Devices
{
	static Devices *instance;

	DevicesOnOffImpl devices_onoff;
	DevicesHTImpl devices_ht;
	DevicesPassiveImpl devices_passive;

	public:
		Devices();

		static Devices *GetInstance() { return instance; }

		std::string IDToName(int id);

		void Reload();
		void Unload();
};

}

#endif


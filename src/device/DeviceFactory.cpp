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

#include <device/DeviceFactory.hpp>
#include <device/electrical/DeviceTimeRange.hpp>
#include <device/electrical/DeviceHeater.hpp>
#include <device/electrical/DeviceCMV.hpp>
#include <device/electrical/DeviceHWS.hpp>
#include <device/electrical/DevicePassive.hpp>
#include <device/electrical/DevicePV.hpp>
#include <device/electrical/DeviceGrid.hpp>
#include <device/weather/DeviceWind.hpp>
#include <device/weather/DeviceHTWifi.hpp>
#include <device/weather/DeviceHTBluetooth.hpp>
#include <device/DeviceObserver.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;

namespace device
{

Device *DeviceFactory::Get(int id, const string &name, const string &type, const configuration::Json &config)
{
	if(type=="timerange")
		return new DeviceTimeRange(id, name, config);
	else if(type=="heater")
		return new DeviceHeater(id, name, config);
	else if(type=="cmv")
		return new DeviceCMV(id, name, config);
	else if(type=="hws")
		return new DeviceHWS(id, name, config);
	else if(type=="passive")
		return new DevicePassive(id, name, config);
	else if(type=="ht")
		return new DeviceHTWifi(id, name, config);
	else if(type=="htmini")
		return new DeviceHTBluetooth(id, name, config);
	else if(type=="wind")
		return new DeviceWind(id, name, config);
	else if(type=="grid")
		return new DeviceGrid(id, name, config);
	else if(type=="pv")
		return new DevicePV(id, name, config);
	else
		throw invalid_argument("Invalid device type « " + type + " »");
}

void DeviceFactory::CheckConfig(const string &type, const configuration::Json &config)
{
	if(type=="timerange")
		return DeviceTimeRange::CheckConfig(config);
	else if(type=="heater")
		return DeviceHeater::CheckConfig(config);
	else if(type=="cmv")
		return DeviceCMV::CheckConfig(config);
	else if(type=="hws")
		return DeviceHWS::CheckConfig(config);
	else if(type=="passive")
		return DevicePassive::CheckConfig(config);
	else if(type=="ht")
		return DeviceHTWifi::CheckConfig(config);
	else if(type=="htmini")
		return DeviceHTBluetooth::CheckConfig(config);
	else if(type=="wind")
		return DeviceWind::CheckConfig(config);
	else if(type=="grid")
		return DeviceGrid::CheckConfig(config);
	else if(type=="pv")
		return DevicePV::CheckConfig(config);
	else
		throw invalid_argument("Invalid device type « " + type + " »");
}

}

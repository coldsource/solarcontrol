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
#include <device/electrical/DeviceCooler.hpp>
#include <device/electrical/DeviceCMV.hpp>
#include <device/electrical/DeviceHWS.hpp>
#include <device/electrical/DevicePassive.hpp>
#include <device/electrical/DevicePV.hpp>
#include <device/electrical/DeviceGrid.hpp>
#include <device/electrical/DeviceBattery.hpp>
#include <device/weather/DeviceWind.hpp>
#include <device/weather/DeviceHTWifi.hpp>
#include <device/weather/DeviceHTBluetooth.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;

namespace device
{

shared_ptr<Device> DeviceFactory::Get(int id, const string &name, const string &type, const configuration::Json &config)
{
	shared_ptr<Device> dev;

	if(type=="timerange")
		dev = make_shared<DeviceTimeRange>(id);
	else if(type=="heater")
		dev = make_shared<DeviceHeater>(id);
	else if(type=="cooler")
		dev = make_shared<DeviceCooler>(id);
	else if(type=="cmv")
		dev = make_shared<DeviceCMV>(id);
	else if(type=="hws")
		dev = make_shared<DeviceHWS>(id);
	else if(type=="passive")
		dev = make_shared<DevicePassive>(id);
	else if(type=="ht")
		dev = make_shared<DeviceHTWifi>(id);
	else if(type=="htmini")
		dev = make_shared<DeviceHTBluetooth>(id);
	else if(type=="wind")
		dev = make_shared<DeviceWind>(id);
	else if(type=="grid")
		dev = make_shared<DeviceGrid>(id);
	else if(type=="pv")
		dev = make_shared<DevicePV>(id);
	else if(type=="battery")
		dev = make_shared<DeviceBattery>(id);
	else
		throw invalid_argument("Invalid device type « " + type + " »");

	dev->Reload(name, config);

	return dev;
}

void DeviceFactory::CheckConfig(const string &type, const configuration::Json &config)
{
	if(type=="timerange")
		return DeviceTimeRange::CheckConfig(config);
	else if(type=="heater")
		return DeviceHeater::CheckConfig(config);
	else if(type=="cooler")
		return DeviceCooler::CheckConfig(config);
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
	else if(type=="battery")
		return DeviceBattery::CheckConfig(config);
	else
		throw invalid_argument("Invalid device type « " + type + " »");
}

}

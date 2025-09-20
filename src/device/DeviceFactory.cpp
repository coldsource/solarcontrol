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
#include <device/electrical/TimeRange.hpp>
#include <device/electrical/Heater.hpp>
#include <device/electrical/Cooler.hpp>
#include <device/electrical/CMV.hpp>
#include <device/electrical/HWS.hpp>
#include <device/electrical/Passive.hpp>
#include <device/electrical/PV.hpp>
#include <device/electrical/Grid.hpp>
#include <device/electrical/Battery.hpp>
#include <device/electrical/BatteryPassive.hpp>
#include <device/weather/Wind.hpp>
#include <device/weather/HTWifi.hpp>
#include <device/weather/HTBluetooth.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;

namespace device
{

shared_ptr<Device> DeviceFactory::Get(int id, const string &name, const string &type, const configuration::Json &config)
{
	shared_ptr<Device> dev;

	if(type=="timerange")
		dev = make_shared<TimeRange>(id);
	else if(type=="heater")
		dev = make_shared<Heater>(id);
	else if(type=="cooler")
		dev = make_shared<Cooler>(id);
	else if(type=="cmv")
		dev = make_shared<CMV>(id);
	else if(type=="hws")
		dev = make_shared<HWS>(id);
	else if(type=="passive")
		dev = make_shared<Passive>(id);
	else if(type=="ht")
		dev = make_shared<HTWifi>(id);
	else if(type=="htmini")
		dev = make_shared<HTBluetooth>(id);
	else if(type=="wind")
		dev = make_shared<Wind>(id);
	else if(type=="grid")
		dev = make_shared<Grid>(id);
	else if(type=="pv")
		dev = make_shared<PV>(id);
	else if(type=="battery")
		dev = make_shared<Battery>(id);
	else if(type=="battery-passive")
		dev = make_shared<BatteryPassive>(id);
	else
		throw invalid_argument("Invalid device type « " + type + " »");

	dev->Reload(name, config);

	return dev;
}

void DeviceFactory::CheckConfig(const string &type, const configuration::Json &config)
{
	if(type=="timerange")
		return TimeRange::CheckConfig(config);
	else if(type=="heater")
		return Heater::CheckConfig(config);
	else if(type=="cooler")
		return Cooler::CheckConfig(config);
	else if(type=="cmv")
		return CMV::CheckConfig(config);
	else if(type=="hws")
		return HWS::CheckConfig(config);
	else if(type=="passive")
		return Passive::CheckConfig(config);
	else if(type=="ht")
		return HTWifi::CheckConfig(config);
	else if(type=="htmini")
		return HTBluetooth::CheckConfig(config);
	else if(type=="wind")
		return Wind::CheckConfig(config);
	else if(type=="grid")
		return Grid::CheckConfig(config);
	else if(type=="pv")
		return PV::CheckConfig(config);
	else if(type=="battery")
		return Battery::CheckConfig(config);
	else if(type=="battery-passive")
		return BatteryPassive::CheckConfig(config);
	else
		throw invalid_argument("Invalid device type « " + type + " »");
}

}

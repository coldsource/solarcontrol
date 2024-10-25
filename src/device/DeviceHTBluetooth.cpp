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


#include <device/DeviceHTBluetooth.hpp>

using namespace std;

namespace device
{

DeviceHTBluetooth::DeviceHTBluetooth(unsigned int id, const string &name, const nlohmann::json &config): DeviceHT(id, name)
{
	check_config_parameters(config, {"ble_addr"});

	ctrl = new control::HTBluetooth(config["ble_addr"]);
}

DeviceHTBluetooth::~DeviceHTBluetooth()
{
	delete ctrl;
}

double DeviceHTBluetooth::GetTemperature() const
{
	return ctrl->GetTemperature();
}

double DeviceHTBluetooth::GetHumidity() const
{
	return ctrl->GetHumidity();
}

}


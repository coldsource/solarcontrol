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

#include <device/Device.hpp>

using namespace std;
using nlohmann::json;

namespace device {

Device::Device(unsigned int id, const string &name)
{
	this->id = id;
	this->name = name;
}

void Device::check_config_parameters(const json &config, const vector<string> &names)
{
	for(auto name : names)
	{
		if(!config.contains(name))
			throw runtime_error("Unable to load configuration for device : missing parameter « " + name + " »");
	}
}

}

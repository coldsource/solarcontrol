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

#include <thread/LCD.hpp>
#include <display/LCDDisplay.hpp>
#include <energy/GlobalMeter.hpp>
#include <device/Devices.hpp>
#include <device/electrical/Electrical.hpp>
#include <configuration/Configuration.hpp>
#include <configuration/ConfigurationPart.hpp>

#include <string>
#include <iostream>
#include <memory>

using namespace std;
using display::LCDDisplay;

namespace thread
{

LCD::LCD()
{
	set_enabled(configuration::Configuration::FromType("solarcontrol")->GetBool("display.lcd.enable"));

	start();
}

string LCD::format_power(double p) const
{
	if(std::abs(p)<1000)
		return to_string((int)p) + "W";

	char f[32];
	snprintf(f, 32, "%.01f", p/1000);
	return string(f) + "kW";
}

string LCD::format_energy(double e) const
{
	if(std::abs(e)<1000)
		return to_string((int)e) + "Wh";

	char f[32];
	snprintf(f, 32, "%.01f", e/1000);
	return string(f) + "kWh";
}

void LCD::main()
{
	auto global = energy::GlobalMeter::GetInstance();
	auto config = configuration::Configuration::FromType("solarcontrol");

	string path = config->Get("display.lcd.path");
	int address = config->GetInt("display.lcd.address");
	size_t line_size = config->GetUInt("display.lcd.linesize");
	bool debug = config->GetBool("display.lcd.debug");

	unique_ptr<LCDDisplay> lcd;
	if(!debug)
	{
		lcd.reset(new LCDDisplay(path, address, line_size));
		lcd->Clear();
	}

	while(true)
	{
		// Get top consuming device
		string max_name;
		double max_power = 0;
		double max_energy = 0;

		{
			for(auto device : device::Devices::Get<device::Electrical>())
			{
				if(device->GetID()==DEVICE_ID_GRID || device->GetID()==DEVICE_ID_PV)
					continue; // Exclude special devices

				if(device->GetPower()>max_power)
				{
					max_name = device->GetName();
					max_power = device->GetPower();
					max_energy = device->GetEnergyConsumption();
				}
			}
		}

		string l1 = "Grid " + format_power(global->GetGridPower()) + " (" + format_energy(global->GetGridEnergy()) + ")";
		string l2 = "PV " + format_power(global->GetPVPower()) + " (" + format_power(global->GetGrossAvailablePower()) + ")";
		string l3;
		string l4;
		if(max_power>0)
		{
			l3 = max_name.size()<line_size?max_name:(max_name.substr(0, line_size-3) + "...");
			l4 = format_power(max_power) + " (" + format_energy(max_energy) + ")";
		}

		if(!debug)
		{
			lcd->Home();
			lcd->WriteLine(1, l1);
			lcd->WriteLine(2, l2);
			lcd->WriteLine(3, l3);
			lcd->WriteLine(4, l4);
		}
		else
		{
			// In debug mode we simply output on stdout
			cout<<l1<<endl;
			cout<<l2<<endl;
			cout<<l3<<endl;
			cout<<l4<<endl;
			cout<<endl;
		}

		if(!wait(2))
			return;
	}
}

}

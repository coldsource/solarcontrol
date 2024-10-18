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
#include <configuration/ConfigurationSolarControl.hpp>

#include <string>

using namespace std;

namespace thread
{

LCD::LCD()
{
	set_enabled(configuration::ConfigurationSolarControl::GetInstance()->GetBool("display.lcd.enable"));

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
	auto config = configuration::ConfigurationSolarControl::GetInstance();

	string path = config->Get("display.lcd.path");
	int address = 0x27;//config->Get("display.lcd.address");
	int line_size = config->GetInt("display.lcd.linesize");

	display::LCDDisplay lcd(path, address, line_size);
	lcd.Clear();

	while(true)
	{
		string l1 = "Grid " + format_power(global->GetGridPower()) + " (" + format_power(global->GetPower()) + ")";
		string l2 = "PV " + format_power(global->GetPVPower()) + " (" + format_power(global->GetNetAvailablePower()) + ")";
		string l3 = "" + format_energy(global->GetGridEnergy()) + " / " + format_energy(global->GetExportedEnergy());
		string l4 = "HWS " + format_energy(global->GetHWSEnergy()) + " (" + format_power(global->GetHWSPower()) + ")";
		lcd.Home();
		lcd.WriteLine(1, l1);
		lcd.WriteLine(2, l2);
		lcd.WriteLine(3, l3);
		lcd.WriteLine(4, l4);

		if(!wait(2))
			return;
	}
}

}

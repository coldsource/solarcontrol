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
#include <energy/Global.hpp>
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

void LCD::main()
{
	auto global = energy::Global::GetInstance();
	auto config = configuration::ConfigurationSolarControl::GetInstance();

	string path = config->Get("display.lcd.path");
	int address = 0x27;//config->Get("display.lcd.address");
	int line_size = config->GetInt("display.lcd.linesize");

	display::LCDDisplay lcd(path, address, line_size);
	lcd.Clear();

	while(true)
	{
		string l1 = to_string((int)global->GetPower());
		lcd.Home();
		lcd.WriteLine(1, l1);

		if(!wait(2))
			return;
	}
}

}

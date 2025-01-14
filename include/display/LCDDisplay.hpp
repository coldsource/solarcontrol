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

#ifndef __DISPLAY_LCDDISPLAY_HPP__
#define __DISPLAY_LCDDISPLAY_HPP__

#include <string>

namespace display {

class LCDDisplay
{
	int fd = -1;
	size_t line_size;
	int backlight;

	public:
		LCDDisplay(const std::string &device_path, int i2c_address, size_t line_size);
		~LCDDisplay();

		void Clear();
		void Home();
		void SetBacklight(bool onoff);
		void WriteLine(int line, const std::string &str);

	protected:
		void i2c_send_byte(unsigned char data);
		void lcd_strobe(unsigned char data);
		void lcd_write_four_bits(unsigned char data);
		void lcd_write(unsigned char cmd, unsigned char mode = 0);
};

}

#endif



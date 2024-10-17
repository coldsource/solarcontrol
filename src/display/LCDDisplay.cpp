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
 *
 * Driver for controlling LCD 20x4 Liquid Crytal for Raspberry PI
 * Using i2c to communicate with the device
 *
 * Tested on Raspberry Pi 3
 */

#include <display/LCDDisplay.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <stdexcept>

using namespace std;

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0b00000100 // Enable bit
#define Rw 0b00000010 // Read/Write bit
#define Rs 0b00000001 // Register select bit

namespace display
{

LCDDisplay::LCDDisplay(const std::string &device_path, int i2c_address, int line_size)
{
	this->line_size = line_size;
	backlight = LCD_BACKLIGHT;

	fd = open(device_path.c_str(), O_RDWR);
	if(fd<0)
		throw runtime_error("Unable to open i2c device « " + device_path + " »");

	ioctl(fd, I2C_SLAVE, i2c_address);

	lcd_write(0x03);
	lcd_write(0x03);
	lcd_write(0x03);
	lcd_write(0x02);

	lcd_write(LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE);
	lcd_write(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
	usleep(200000);
}

LCDDisplay::~LCDDisplay()
{
	close(fd);
}

void LCDDisplay::Clear()
{
	lcd_write(LCD_CLEARDISPLAY);
}

void LCDDisplay::Home()
{
	lcd_write(LCD_ENTRYMODESET | LCD_ENTRYLEFT);
}

void LCDDisplay::SetBacklight(bool onoff)
{
	if(onoff)
		backlight = LCD_BACKLIGHT;
	else
		backlight = LCD_NOBACKLIGHT;
}

void LCDDisplay::WriteLine(int line, const std::string &str)
{
	if(line<=0 || line>4)
		throw invalid_argument("Line number must be between 1 and 4");

	if(str.size()>line_size)
		throw invalid_argument("Line is too long : « " + str + " »");

	unsigned char line_id[] = {0x80, 0xC0, 0x94, 0xD4};
	lcd_write(line_id[line-1]);

	int i;
	for(i=0; i<str.size(); i++)
		lcd_write(str[i], Rs);
	for(; i<line_size; i++)
		lcd_write(' ', Rs);
}

void LCDDisplay::i2c_send_byte(unsigned char data)
{
	if(fd==-1)
		throw runtime_error("LCD device not connected");

	write(fd, &data, 1);
}

void LCDDisplay::lcd_strobe(unsigned char data)
{
	i2c_send_byte(data | En);
	usleep(5000);
	i2c_send_byte(data);
	usleep(2000);
}

void LCDDisplay::lcd_write_four_bits(unsigned char data)
{
	data |= backlight;
	i2c_send_byte(data);
	lcd_strobe(data);
}

void LCDDisplay::lcd_write(unsigned char cmd, unsigned char mode)
{
	lcd_write_four_bits(mode | (cmd & 0xF0));
	lcd_write_four_bits(mode | ((cmd <<4) & 0xF0));
}

}

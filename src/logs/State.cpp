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

#include <logs/State.hpp>
#include <database/DB.hpp>

#include <string>

using namespace std;
using database::DB;

namespace logs
{

void State::LogModeChange(int device_id, en_mode mode)
{
	DB db;

	string mode_str = (mode==automatic)?"automatic":"manual";

	db.Query("INSERT INTO  t_log_state(device_id, log_state_mode) VALUES(%i, %s)"_sql <<device_id<<mode_str);
}

void State::LogStateChange(int device_id, en_mode mode, bool state)
{
	DB db;

	string mode_str = (mode==automatic)?"automatic":"manual";

	db.Query("INSERT INTO  t_log_state(device_id, log_state_mode, log_state) VALUES(%i, %s, %i)"_sql <<device_id<<mode_str<<state);

}

}

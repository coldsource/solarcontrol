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

#include <device/electrical/Grid.hpp>
#include <device/Devices.hpp>
#include <sensor/input/Factory.hpp>
#include <sensor/input/Input.hpp>
#include <configuration/Json.hpp>
#include <database/DB.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

Grid::Grid(int id):Passive(id, "consumption", "excess") // Override default counter for storing excess
{
}

Grid::~Grid()
{
}

void Grid::CheckConfig(const configuration::Json &conf)
{
	Passive::CheckConfig(conf);

	if(conf.Has("input"))
	{
		conf.Check("input", "object"); // Input is mandatory for Grid
		sensor::input::Factory::CheckConfig(conf.GetObject("input"));
	}

	if(conf.Has("input_grid"))
	{
		conf.Check("input_grid", "object"); // Input is mandatory for Grid
		sensor::input::Factory::CheckConfig(conf.GetObject("input_grid"));
	}
}

void Grid::reload(const configuration::Json &config)
{
	Passive::reload(config);

	if(config.Has("input"))
		add_sensor(sensor::input::Factory::GetFromConfig(config.GetObject("input")), "offpeak_ctrl");

	if(config.Has("input_grid"))
		add_sensor(sensor::input::Factory::GetFromConfig(config.GetObject("input_grid")), "grid_detection");
}

Grid::en_grid_state Grid::string_to_grid_state(const string &str)
{
	if(str=="online")
		return ONLINE;
	else if(str=="offline")
		return OFFLINE;
	return UNKNOWN;
}

string Grid::grid_state_to_string(en_grid_state state)
{
	if(state==ONLINE)
		return "online";
	else if(state==OFFLINE)
		return "offline";
	return "unknown";
}

bool Grid::GetOffPeak() const
{
	unique_lock<recursive_mutex> llock(lock);

	return offpeak;
}

Grid::en_grid_state Grid::GetState() const
{
	unique_lock<recursive_mutex> llock(lock);

	return grid_state;
}

void Grid::SensorChanged(const  sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	if(sensor->GetName()=="offpeak_ctrl")
		offpeak = ((sensor::input::Input *)sensor)->GetState();
	else if(sensor->GetName()=="grid_detection")
		grid_state = ((sensor::input::Input *)sensor)->GetState()?ONLINE:OFFLINE;
	else
		Passive::SensorChanged(sensor); // Forward message
}

json Grid::ToJson() const
{
	unique_lock<recursive_mutex> llock(lock);

	json j_device = Passive::ToJson();
	j_device["grid_state"] = grid_state_to_string(grid_state);
	return j_device;
}

void Grid::CreateInDB()
{
	database::DB db;
	auto res = db.Query("SELECT device_id FROM t_device WHERE device_type='grid'"_sql);
	if(res.FetchRow())
		return; // Already in database

	json config = json::object();
	db.Query("INSERT INTO t_device(device_id, device_type, device_name, device_config) VALUES(%i, 'grid', 'grid', %s)"_sql<<DEVICE_ID_GRID<<config.dump());
}

}

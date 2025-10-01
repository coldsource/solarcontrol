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
#include <sensor/input/InputFactory.hpp>
#include <sensor/input/Input.hpp>
#include <configuration/Json.hpp>
#include <database/DB.hpp>

using namespace std;
using nlohmann::json;
using sensor::input::InputFactory;

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
		InputFactory::CheckConfig(conf.GetObject("input"));
	}
}

void Grid::reload(const configuration::Json &config)
{
	Passive::reload(config);

	if(config.Has("input"))
		add_sensor(InputFactory::GetFromConfig(config.GetObject("input")), "offpeak_ctrl");
}

void Grid::SensorChanged(const  sensor::Sensor *sensor)
{
	unique_lock<recursive_mutex> llock(lock);

	if(sensor->GetName()=="offpeak_ctrl")
		offpeak = ((sensor::input::Input *)sensor)->GetState();
	else
		Passive::SensorChanged(sensor); // Forward message
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

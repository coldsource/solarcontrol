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

#include <device/electrical/DeviceHWS.hpp>
#include <device/Devices.hpp>
#include <configuration/Json.hpp>
#include <meter/MeterFactory.hpp>
#include <database/DB.hpp>
#include <control/ConfigurationControl.hpp>

using namespace std;
using nlohmann::json;

namespace device
{

DeviceHWS::DeviceHWS(unsigned int id, const string &name, const configuration::Json &config):DeviceTimeRange(id, name, config)
{
	min_energy = config.GetInt("min_energy");
	min_energy_for_last = config.GetInt("min_energy_for_last");

	auto ctrl_config = configuration::ConfigurationControl::GetInstance();
	ObserveConfiguration(ctrl_config);
}

DeviceHWS::~DeviceHWS()
{
	auto ctrl_config = configuration::ConfigurationControl::GetInstance();
	StopObserveConfiguration(ctrl_config);
}

void DeviceHWS::ConfigurationChanged(const configuration::Configuration * config)
{
	absence = config->GetBool("control.absence.enabled");
}

void DeviceHWS::CheckConfig(const configuration::Json &conf)
{
	DeviceTimeRange::CheckConfig(conf);

	conf.Check("min_energy", "float");
	conf.Check("min_energy_for_last", "int");

	conf.Check("meter", "object"); // Meter is mandatory for HWS
	meter::MeterFactory::CheckConfig(conf.GetObject("meter"));
}

bool DeviceHWS::WantRemainder(configuration::Json *data_ptr) const
{
	if(absence)
		return false; // No remainder in absence mode

	double last_energy = consumption.GetTotalConsumptionForLast(min_energy_for_last);
	return remainder.IsActive(data_ptr) && last_energy<min_energy;
}

void DeviceHWS::CreateInDB()
{
	database::DB db;
	auto res = db.Query("SELECT device_id FROM t_device WHERE device_type='hws'"_sql);
	if(res.FetchRow())
		return; // Already in database

	json config;
	config["prio"] = 0;

	json control;
	control["type"] = "pro";
	control["ip"] = "";
	control["outlet"] = 0;
	config["control"] = control;

	json meter;
	meter["type"] = "3em";
	meter["mqtt_id"] = "";
	meter["phase"] = "c";
	config["meter"] = meter;

	config["force"] = json::array();
	config["remainder"] = json::array();
	config["min_energy"] = 0;
	config["min_energy_for_last"] = 0;

	db.Query("INSERT INTO t_device(device_id, device_type, device_name, device_config) VALUES(%i, 'hws', 'hws', %s)"_sql<<DEVICE_ID_HWS<<config.dump());
}

}


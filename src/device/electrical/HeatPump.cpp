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

#include <device/electrical/HeatPump.hpp>
#include <device/weather/Weather.hpp>
#include <device/Devices.hpp>
#include <control/BSBLan.hpp>
#include <configuration/Json.hpp>
#include <configuration/Configuration.hpp>
#include <configuration/ConfigurationPart.hpp>
#include <excpt/BSBLan.hpp>
#include <logs/Logger.hpp>

using datetime::Timestamp;
using nlohmann::json;
using namespace std;

namespace device {

HeatPump::HeatPump(int id):
Temperature(id),
update_interval(configuration::Configuration::FromType("solarcontrol")->GetTime("heatpump.sync"))
{
}

void HeatPump::CheckConfig(const configuration::Json &conf)
{
	conf.Check("temperature_offset_slow", "float");
	conf.Check("temperature_offset_fast", "float");
	conf.Check("temperature_eco", "float");
	conf.Check("temperature_comfort", "float");

	Temperature::CheckConfig(conf);
}

void HeatPump::reload(const configuration::Json &config)
{
	Temperature::reload(config);

	temperature_offset_slow = config.GetFloat("temperature_offset_slow");
	temperature_offset_fast = config.GetFloat("temperature_offset_fast");
	temperature_eco = config.GetFloat("temperature_eco");
	temperature_comfort = config.GetFloat("temperature_comfort");

	update_interval.Reset(); // Push new config now
}

bool HeatPump::temp_check_force(double current_temp, double timerange_temp) const
{
	if(absence)
		timerange_temp = absence_temperature; // Use special absence temperature if absent mode is on

	return current_temp<timerange_temp;
}

bool HeatPump::temp_check_offload(double current_temp, double timerange_temp) const
{
	return current_temp<timerange_temp; // In offload mode, we ignore absent mode
}

void HeatPump::SpecificActions()
{
	unique_lock<recursive_mutex> llock(lock);

	auto bsblan = std::dynamic_pointer_cast<control::BSBLan>(ctrl);
	if(bsblan==nullptr)
		return; // Only special BSB Lan controller can handle these actions

	if(!update_interval.Ready())
		return;

	shared_ptr<Weather> ht;
	try
	{
		ht = Devices::GetByID<Weather>(ht_device_id);
	}
	catch(exception &e)
	{
		// Associated thermometer has been removed, fallback to safe mode
		logs::Logger::Log(LOG_NOTICE, "Missing thermometer for device « " + GetName() + " »");
		return;
	}

	try
	{
		double ambient = ht->GetTemperature();
		bsblan->SetAmbientTemperature(ambient);

		double eco_temp = temperature_eco;
		if(absence)
			eco_temp = absence_temperature; // If absent lower temperature of eco mode to avoid switch on
		bsblan->SetEcoSetPoint(eco_temp);

		// Handle special timerange data
		auto tr_data = GetCurrentTimerangeData();

		// Mode
		bool fast_mode = false;
		if(tr_data.Has("speed"))
			fast_mode = (tr_data.GetString("speed")=="fast");

		// Compute setpoint
		double setpoint = temperature_comfort;
		if(tr_data.Has("temperature"))
			setpoint = tr_data.GetFloat("temperature");

		double temperature_offset = fast_mode ? temperature_offset_fast : temperature_offset_slow;

		// Build offset based setpointsetpoint
		double t = ambient + temperature_offset;
		if(t>setpoint)
			t = setpoint;
		bsblan->SetComfortSetPoint(t);
	}
	catch(excpt::BSBLan &e)
	{
		e.Log(LOG_ERR);
	}
	catch(exception &e)
	{
		logs::Logger::Log(LOG_ERR, e.what());
	}
}

}


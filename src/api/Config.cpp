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

#include <api/Config.hpp>
#include <database/DB.hpp>
#include <energy/GlobalMeter.hpp>
#include <thread/DevicesManager.hpp>
#include <configuration/ConfigurationSolarControl.hpp>
#include <energy/ConfigurationEnergy.hpp>
#include <control/ConfigurationControl.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using database::DB;

namespace api
{

json Config::HandleMessage(const string &cmd, const configuration::Json &j_params)
{
	json j_res;

	string module = j_params.GetString("module");

	auto config = configuration::ConfigurationSolarControl::GetInstance();
	configuration::Configuration *config_module;
	if(module=="energy")
		config_module = configuration::ConfigurationEnergy::GetInstance();
	else if(module=="control")
		config_module = configuration::ConfigurationControl::GetInstance();
	else
		throw invalid_argument("Unknow configuration module « " + module + " »");

	if(cmd=="get")
	{
		json j_config_master = json::object();
		for(auto entry : config->GetAll())
			j_config_master[entry.first] = entry.second;

		json j_config = json::object();
		for(auto entry : config_module->GetAll())
			j_config[entry.first] = entry.second;

		j_res = json::object();
		j_res["config_master"] = j_config_master;
		j_res["config"] = j_config;

		return j_res;
	}
	else if(cmd=="set")
	{
		string name = j_params.GetString("name");
		string value = j_params.GetString("value");

		config_module->SetCheck(name, value);

		DB db;
		db.Query("REPLACE INTO t_config(config_name, config_value) VALUES(%s, %s)"_sql<<name<<value);

		return json();
	}
	else if(cmd=="reset")
	{
		string name = j_params.GetString("name");

		string default_value = config->Get(name);
		config_module->Set(name, default_value);

		DB db;
		db.Query("DELETE FROM t_config WHERE config_name=%s"_sql<<name);

		return json(default_value);
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « config »");
}

}


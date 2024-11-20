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

#include <api/Logs.hpp>
#include <database/DB.hpp>
#include <energy/GlobalMeter.hpp>
#include <configuration/Json.hpp>

#include <stdexcept>

using namespace std;
using nlohmann::json;
using database::DB;

namespace api
{

json Logs::HandleMessage(const string &cmd, const configuration::Json &j_params)
{
	json j_res;
	DB db;

	if(cmd=="state")
	{
		DB db;

		auto res = db.Query(" \
			SELECT dev.device_name, ls.log_state_date, ls.log_state_mode, ls.log_state \
			FROM t_log_state ls \
			INNER JOIN t_device dev ON ls.device_id=dev.device_id \
			WHERE ls.log_state IS NOT NULL \
			ORDER BY log_state_date DESC \
			LIMIT 100"_sql);

		j_res = json::array();
		while(res.FetchRow())
		{
			json j_device;
			j_device["device_name"] = res["device_name"];
			j_device["log_state_date"] = res["log_state_date"];
			j_device["log_state_mode"] = res["log_state_mode"];
			j_device["log_state"] = res["log_state"];

			j_res.push_back(j_device);
		}

		return j_res;
	}
	else if(cmd=="energy")
	{
		auto global_meter = energy::GlobalMeter::GetInstance();
		j_res = json::object();

		for(auto grid_consumption : global_meter->GetGridConsumptionHistory())
			j_res[string(grid_consumption.first)]["grid_consumption"] = grid_consumption.second;

		for(auto grid_excess : global_meter->GetGridExcessHistory())
			j_res[string(grid_excess.first)]["grid_excess"] = grid_excess.second;

		for(auto pv_production : global_meter->GetPVProductionHistory())
			j_res[string(pv_production.first)]["pv_production"] = pv_production.second;

		for(auto hws_consumption : global_meter->GetHWSConsumptionHistory())
			j_res[string(hws_consumption.first)]["hws_consumption"] = hws_consumption.second;

		for(auto offpeak_consumption : global_meter->GetOffPeakConsumptionHistory())
			j_res[string(offpeak_consumption.first)]["offpeak_consumption"] = offpeak_consumption.second;

		for(auto peak_consumption : global_meter->GetPeakConsumptionHistory())
			j_res[string(peak_consumption.first)]["peak_consumption"] = peak_consumption.second;

		for(auto hws_forced_consumption : global_meter->GetHWSForcedConsumptionHistory())
			j_res[string(hws_forced_consumption.first)]["hws_forced_consumption"] = hws_forced_consumption.second;

		for(auto hws_offload_consumption : global_meter->GetHWSOffloadConsumptionHistory())
			j_res[string(hws_offload_consumption.first)]["hws_offload_consumption"] = hws_offload_consumption.second;

		return j_res;
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « logs »");
}

}

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
#include <energy/Amount.hpp>
#include <configuration/Json.hpp>
#include <datetime/Date.hpp>
#include <datetime/Month.hpp>
#include <device/Devices.hpp>
#include <device/DevicesOnOff.hpp>
#include <device/DevicesPassive.hpp>

#include <map>
#include <stdexcept>

using namespace std;
using nlohmann::json;
using database::DB;
using datetime::Date;
using datetime::Month;

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
		int months_before = j_params.GetInt("mbefore", -1);
		if(months_before==-1)
		{
			auto global_meter = energy::GlobalMeter::GetInstance();
			j_res = json::object();

			for(auto grid_consumption : global_meter->GetGridConsumptionHistory())
				j_res[string(grid_consumption.first)][DEVICE_NAME_GRID]["consumption"] = grid_consumption.second;

			for(auto grid_excess : global_meter->GetGridExcessHistory())
				j_res[string(grid_excess.first)][DEVICE_NAME_GRID]["excess"] = grid_excess.second;

			for(auto pv_production : global_meter->GetPVProductionHistory())
				j_res[string(pv_production.first)][DEVICE_NAME_PV]["production"] = pv_production.second;

			for(auto hws_consumption : global_meter->GetHWSConsumptionHistory())
				j_res[string(hws_consumption.first)][DEVICE_NAME_HWS]["consumption"] = hws_consumption.second;

			for(auto hws_offload_consumption : global_meter->GetHWSOffloadConsumptionHistory())
				j_res[string(hws_offload_consumption.first)][DEVICE_NAME_HWS]["offload"] = hws_offload_consumption.second;

			for(auto device : device::DevicesOnOff())
			{
				for(auto consumption : device->GetConsumptionHistory())
					j_res[string(consumption.first)][device->GetName()]["consumption"] = consumption.second;

				for(auto offload : device->GetOffloadHistory())
					j_res[string(offload.first)][device->GetName()]["offload"] = offload.second;
			}

			for(auto device : device::DevicesPassive())
			{
				for(auto consumption : device->GetConsumptionHistory())
					j_res[string(consumption.first)][device->GetName()]["consumption"] = consumption.second;

				for(auto offload : device->GetOffloadHistory())
					j_res[string(offload.first)][device->GetName()]["offload"] = offload.second;
			}

			return j_res;
		}
		else
		{
			DB db;

			Month m;
			Month from = m - months_before;
			Month to = from + 1;

			auto res = db.Query(" \
				SELECT log_energy_date, device_id, log_energy_type, log_energy, log_energy_peak, log_energy_offpeak \
				FROM t_log_energy  detail \
				WHERE log_energy_date >= %s \
				AND log_energy_date < %s \
			"_sql<<string(from)<<string(to));

			j_res = json::object();
			while(res.FetchRow())
			{
				string date = res["log_energy_date"];
				int device_id = res["device_id"];
				string device_name = device::Devices::GetInstance()->IDToName(device_id);
				string type = res["log_energy_type"];
				energy::Amount amount(res["log_energy"], res["log_energy_peak"], res["log_energy_offpeak"]);

				if(!j_res.contains(date))
					j_res[date] = json::object();

				if(!j_res[date].contains(device_name))
					j_res[date][device_name] = json::object();

				j_res[date][device_name][type] = amount;
			}

			return j_res;
		}
	}
	else if(cmd=="energydetail")
	{
		DB db;
		database::Query query;

		string day_str = j_params.GetString("day", "");

		Date from;

		if(day_str!="")
			from = Date(day_str);

		Date to = from + 1;

		query = " \
			SELECT detail.device_id, detail.log_energy_detail_date, detail.log_energy_detail_type, detail.log_energy, detail.log_energy_peak, detail.log_energy_offpeak \
			FROM t_log_energy_detail detail \
			WHERE detail.log_energy_detail_date >= %s \
			AND log_energy_detail_date < %s \
		"_sql<<string(from)<<string(to);

		auto res = db.Query(query);

		j_res = json::object();
		while(res.FetchRow())
		{
			int device_id = res["device_id"];
			string date = res["log_energy_detail_date"];
			string type = res["log_energy_detail_type"];
			string device_name = device::Devices::GetInstance()->IDToName(device_id);
			energy::Amount amount(res["log_energy"], res["log_energy_peak"], res["log_energy_offpeak"]);

			if(!j_res.contains(date))
				j_res[date] = json::object();

			if(!j_res[date].contains(device_name))
				j_res[date][device_name] = json::object();

			j_res[date][device_name][type] = amount;
		}

		return j_res;
	}
	else if(cmd=="ht")
	{
		DB db;

		int device_id = j_params.GetInt("device_id");
		string day_str = j_params.GetString("day", "");
		int months_before = j_params.GetInt("mbefore", -2);

		database::Query query;
		if(day_str!="")
		{
			Date from = Date(day_str);
			Date to = from + 1;

			query = " \
				SELECT htw.log_ht_date, htw.log_ht_min_h, htw.log_ht_max_h, htw.log_ht_min_t, htw.log_ht_max_t, htw.log_ht_min_w, htw.log_ht_max_w \
				FROM t_log_htw htw \
				WHERE htw.device_id=%i \
				AND htw.log_ht_date >= %s \
				AND htw.log_ht_date < %s \
				ORDER BY htw.log_ht_date \
			"_sql << device_id << string(from) << string(to);
		}
		else if(months_before!=-2)
		{
			Month m;
			Month from;
			Month to;

			if(months_before>=0)
			{
				from = m - months_before;
				to = from + 1;
			}
			else
			{
				from = m - 1;
				to = m + 1;
			}

			query = " \
				SELECT DATE(htw.log_ht_date) AS log_ht_date, MIN(htw.log_ht_min_h) AS log_ht_min_h, MAX(htw.log_ht_max_h) AS log_ht_max_h, MIN(htw.log_ht_min_t) AS log_ht_min_t, MAX(htw.log_ht_max_t) AS log_ht_max_t, MIN(htw.log_ht_min_w) AS log_ht_min_w, MAX(htw.log_ht_max_w) AS log_ht_max_w \
				FROM t_log_htw htw \
				WHERE htw.device_id=%i \
				AND htw.log_ht_date >= %s \
				AND htw.log_ht_date < %s \
				GROUP BY DATE(htw.log_ht_date) \
				ORDER BY log_ht_date \
			"_sql << device_id << string(from) << string(to);
		}
		else
		{
			query = " \
				SELECT htw.log_ht_date, htw.log_ht_min_h, htw.log_ht_max_h, htw.log_ht_min_t, htw.log_ht_max_t, htw.log_ht_min_w, htw.log_ht_max_w \
				FROM t_log_htw htw \
				WHERE htw.device_id=%i \
				AND htw.log_ht_date >= DATE_SUB( NOW() , INTERVAL 1 DAY ) \
				ORDER BY htw.log_ht_date \
			"_sql << device_id;
		}

		auto res = db.Query(query);

		j_res = json::object();
		while(res.FetchRow())
		{
			string date = res["log_ht_date"];
			if(!j_res.contains(date))
				j_res[date] = json::object();

			json &j_entry = j_res[date];
			if(!res["log_ht_min_h"].IsNull())
				j_entry["hmin"] = (double)res["log_ht_min_h"];
			if(!res["log_ht_max_h"].IsNull())
				j_entry["hmax"] = (double)res["log_ht_max_h"];
			if(!res["log_ht_min_t"].IsNull())
				j_entry["tmin"] = (double)res["log_ht_min_t"];
			if(!res["log_ht_max_t"].IsNull())
				j_entry["tmax"] = (double)res["log_ht_max_t"];
			if(!res["log_ht_min_w"].IsNull())
				j_entry["wmin"] = (double)res["log_ht_min_w"];
			if(!res["log_ht_max_w"].IsNull())
				j_entry["wmax"] = (double)res["log_ht_max_w"];
		}

		return j_res;
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « logs »");
}

}

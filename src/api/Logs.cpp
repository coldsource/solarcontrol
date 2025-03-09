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
#include <energy/Amount.hpp>
#include <configuration/Json.hpp>
#include <datetime/Date.hpp>
#include <datetime/DateTime.hpp>
#include <datetime/Month.hpp>
#include <device/Devices.hpp>
#include <device/electrical/DeviceElectrical.hpp>
#include <device/electrical/DeviceGrid.hpp>
#include <device/electrical/DevicePV.hpp>

#include <map>
#include <stdexcept>

using namespace std;
using nlohmann::json;
using database::DB;
using datetime::Date;
using datetime::DateTime;
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
			j_res = json::object();

			device::Devices devices;
			for(auto device : devices.GetElectrical())
			{
				for(auto consumption : device->GetConsumptionHistory())
					j_res[string(consumption.first)][device->GetName()]["consumption"] = consumption.second;

				if(device->GetType()=="grid")
				{
					for(auto excess : ((device::DeviceGrid *)device)->GetExcessHistory())
						j_res[string(excess.first)][device->GetName()]["excess"] = excess.second;
				}

				if(device->GetType()=="pv")
				{
					for(auto production : ((device::DevicePV *)device)->GetProductionHistory())
						j_res[string(production.first)][device->GetName()]["production"] = production.second;
				}

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
				string device_name = device::Devices().IDToName(device_id);
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
			string device_name = device::Devices().IDToName(device_id);
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
		if(months_before!=-2)
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
				SELECT DATE(ht.log_ht_date) AS log_ht_date, MIN(ht.log_ht_min_h) AS log_ht_min_h, MAX(ht.log_ht_max_h) AS log_ht_max_h, MIN(ht.log_ht_min_t) AS log_ht_min_t, MAX(ht.log_ht_max_t) AS log_ht_max_t \
				FROM t_log_ht ht \
				WHERE ht.device_id=%i \
				AND ht.log_ht_date >= %s \
				AND ht.log_ht_date < %s \
				GROUP BY DATE(ht.log_ht_date) \
				ORDER BY log_ht_date \
			"_sql << device_id << string(from) << string(to);
		}
		else
		{
			string from_str;
			string to_str;

			if(day_str!="")
			{
				Date from = Date(day_str);
				from_str = from;
				to_str = from + 1;
			}
			else
			{
				DateTime to;
				to_str = to;
				from_str = to - 86400;
			}

			query = " \
				SELECT log_ht_date, log_ht_min_h, log_ht_max_h, log_ht_min_t, log_ht_max_t \
				FROM t_log_ht \
				WHERE device_id=%i \
				AND log_ht_date >= %s \
				AND log_ht_date < %s \
				ORDER BY log_ht_date \
			"_sql << device_id << from_str << to_str;
		}

		auto res = db.Query(query);

		j_res = json::object();
		while(res.FetchRow())
		{
			string date = res["log_ht_date"];
			if(!j_res.contains(date))
				j_res[date] = json::object();

			json &j_entry = j_res[date];
			j_entry["hmin"] = (double)res["log_ht_min_h"];
			j_entry["hmax"] = (double)res["log_ht_max_h"];
			j_entry["tmin"] = (double)res["log_ht_min_t"];
			j_entry["tmax"] = (double)res["log_ht_max_t"];
		}

		return j_res;
	}
	else if(cmd=="wind")
	{
		DB db;

		string from_str;
		string to_str;

		int device_id = j_params.GetInt("device_id");
		string day_str = j_params.GetString("day");

		if(day_str!="")
		{
			Date from = Date(day_str);
			from_str = from;
			to_str = from + 1;
		}
		else
		{
			DateTime to;
			to_str = to;
			from_str = to - 86400;
		}

		database::Query query = " \
			SELECT log_wind_date, log_wind_min, log_wind_max, log_wind_avg \
			FROM t_log_wind \
			WHERE device_id=%i \
			AND log_wind_date >= %s \
			AND log_wind_date < %s \
			ORDER BY log_wind_date \
		"_sql << device_id << from_str << to_str;

		auto res = db.Query(query);

		j_res = json::object();
		while(res.FetchRow())
		{
			string date = res["log_wind_date"];
			if(!j_res.contains(date))
				j_res[date] = json::object();

			json &j_entry = j_res[date];
			j_entry["wmin"] = (double)res["log_wind_min"];
			j_entry["wmax"] = (double)res["log_wind_max"];
			j_entry["wavg"] = (double)res["log_wind_avg"];
		}

		return j_res;
	}

	throw invalid_argument("Unknown command « " + cmd + " » in module « logs »");
}

}

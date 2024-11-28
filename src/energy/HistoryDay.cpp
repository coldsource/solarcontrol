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

#include <energy/HistoryDay.hpp>
#include <database/DB.hpp>
#include <configuration/ConfigurationSolarControl.hpp>

using namespace std;
using datetime::Date;
using database::DB;

namespace energy
{

HistoryDay::HistoryDay(const std::string &type)
:History(configuration::ConfigurationSolarControl::GetInstance()->GetInt("core.history.maxdays"), type)
{
	if(type=="" || type=="device")
		return;

	DB db;

	Date period_ago = Date() - retention;
	auto res = db.Query("SELECT log_energy_date, log_energy FROM t_log_energy WHERE log_energy_type=%s AND log_energy_date>=%s"_sql <<type<<std::string(period_ago));
	while(res.FetchRow())
		history[Date(res["log_energy_date"])] = (double)res["log_energy"];
}

HistoryDay::~HistoryDay()
{
	if(type=="" || type=="device")
		return;

	save();
}

void HistoryDay::save()
{
	for(auto it=history.begin(); it!=history.end(); ++it)
		store_entry(it->first, it->second);
}

void HistoryDay::store_entry(const datetime::Date period, double value)
{
	if(type=="" || type=="device")
		return;

	DB db;

	db.Query(
		"REPLACE INTO t_log_energy(log_energy_date, log_energy_type, log_energy) VALUES(%s, %s, %f)"_sql
		<<std::string(period)<<type<<value
	);
}

}


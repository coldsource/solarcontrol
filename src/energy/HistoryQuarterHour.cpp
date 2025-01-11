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

#include <energy/HistoryQuarterHour.hpp>
#include <database/DB.hpp>
#include <thread/HistorySync.hpp>

using namespace std;
using datetime::QuarterHour;
using database::DB;

namespace energy
{

HistoryQuarterHour::HistoryQuarterHour(int device_id, const std::string &type)
:History(0), type(type), device_id(device_id)
{
	if(type=="")
		return;

	DB db;

	QuarterHour period_ago = QuarterHour() - retention;
	auto res = db.Query("SELECT log_energy_detail_date, log_energy, log_energy_peak, log_energy_offpeak FROM t_log_energy_detail WHERE device_id=%i AND log_energy_detail_type=%s AND log_energy_detail_date>=%s"_sql <<device_id<<type<<std::string(period_ago));
	while(res.FetchRow())
		history[QuarterHour(res["log_energy_detail_date"])] = Amount((double)res["log_energy"], (double)res["log_energy_peak"], (double)res["log_energy_offpeak"]);

	::thread::HistorySync::GetInstance()->Register(this);
}

HistoryQuarterHour::~HistoryQuarterHour()
{
	::thread::HistorySync::GetInstance()->Unregister(this);

	if(type=="")
		return;

	save();
}

void HistoryQuarterHour::save()
{
	for(auto it=history.begin(); it!=history.end(); ++it)
		store_entry(it->first, it->second);
}

void HistoryQuarterHour::store_entry(const QuarterHour period, Amount value)
{
	if(type=="")
		return;

	DB db;

	db.Query(
		"REPLACE INTO t_log_energy_detail(log_energy_detail_date, device_id, log_energy_detail_type, log_energy, log_energy_peak, log_energy_offpeak) VALUES(%s, %i, %s, %f, %f, %f)"_sql
		<<std::string(period)<<device_id<<type<<value.GetEnergy()<<value.GetEnergyPeak()<<value.GetEnergyOffPeak()
	);
}

}



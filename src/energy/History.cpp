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

#include <energy/History.hpp>
#include <database/DB.hpp>
#include <configuration/ConfigurationSolarControl.hpp>

using namespace std;
using datetime::Date;

namespace energy
{

History::History(const string &type): type(type)
{
	retention_days = configuration::ConfigurationSolarControl::GetInstance()->GetInt("core.history.maxdays");

	if(type=="")
		return;

	database::DB db;

	Date days_ago = Date() - retention_days;
	auto res = db.Query("SELECT log_energy_date, log_energy FROM t_log_energy WHERE log_energy_type=%s AND log_energy_date>=%s"_sql <<type<<string(days_ago));
	while(res.FetchRow())
		history[Date(res["log_energy_date"])] = (double)res["log_energy"];
}

History::~History()
{
	save();
}

void History::purge(int ndays)
{
	Date days_ago = Date() - ndays;

	auto it = history.begin();
	while(it!=history.end())
	{
		if(it->first<days_ago)
		{
			if(type!="")
			{
				database::DB db;

				db.Query(
					"REPLACE INTO t_log_energy(log_energy_date, log_energy_type, log_energy) VALUES(%s, %s, %f)"_sql
					<<string(it->first)<<type<<it->second
				);
			}

			it = history.erase(it);
		}
		else
			++it;
	}
}

void History::Set(double energy)
{
	Date today;
	history[today] = energy;
}

void History::Add(double energy)
{
	Date today;
	if(!history.contains(today))
		history[today] = energy;
	else
		history[today] += energy;

	purge(retention_days);
}

void History::save()
{
	if(type=="")
		return;

	database::DB db;

	for(auto it=history.begin(); it!=history.end(); ++it)
	{
		db.Query(
			"REPLACE INTO t_log_energy(log_energy_date, log_energy_type, log_energy) VALUES(%s, %s, %f)"_sql
			<<string(it->first)<<type<<it->second
		);
	}
}

double History::GetTotalForLast(int ndays) const
{
	Date start_date = Date() - ndays;
	double sum = 0;

	for(auto it=history.begin(); it!=history.end(); ++it)
	{
		if(it->first<start_date)
			continue;

		sum += it->second;
	}

	return sum;
}

double History::GetTotalForToday() const
{
	Date today;

	auto it = history.find(today);
	if(it==history.end())
		return 0;

	return it->second;
}

}

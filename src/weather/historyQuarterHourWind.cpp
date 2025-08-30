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

#include <weather/HistoryQuarterHourWind.hpp>
#include <database/DB.hpp>
#include <thread/HistorySync.hpp>


using namespace std;

using database::DB;
using datetime::QuarterHour;
using stat::MinMaxAvg;

namespace weather {

HistoryQuarterHourWind::HistoryQuarterHourWind(int device_id)
:History(0), device_id(device_id)
{
	DB db;

	QuarterHour period_ago = QuarterHour() - retention;
	auto res = db.Query(" \
		SELECT log_wind_date, log_wind_min, log_wind_max, log_wind_avg \
		FROM t_log_wind  \
		WHERE device_id=%i \
		AND log_wind_date>=%s"_sql
		<<device_id<<string(period_ago)
	);

	while(res.FetchRow())
	{
		MinMaxAvg<double> wind(res["log_wind_min"], res["log_wind_max"], res["log_wind_avg"]);
		history.insert(pair<QuarterHour, MinMaxAvg<double>>(QuarterHour(res["log_wind_date"]), wind));
	}

	::thread::HistorySync::GetInstance()->Register(this);
}

HistoryQuarterHourWind::~HistoryQuarterHourWind()
{
	::thread::HistorySync::GetInstance()->Unregister(this);

	save();
}

void HistoryQuarterHourWind::store_entry(const QuarterHour period, MinMaxAvg<double> value)
{
	database::DB db;

	db.Query(" \
		REPLACE INTO t_log_wind(log_wind_date, device_id, log_wind_min, log_wind_max, log_wind_avg) \
		VALUES(%s, %i, %f, %f, %f)"_sql
		<<string(period)<<device_id<<value.GetMin()<<value.GetMax()<<value.GetAvg()
	);
}

void HistoryQuarterHourWind::save()
{
	for(auto it=history.begin(); it!=history.end(); ++it)
		store_entry(it->first, it->second);
}

}


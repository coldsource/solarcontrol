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

#include <weather/HistoryQuarterHourHT.hpp>
#include <database/DB.hpp>
#include <thread/HistorySync.hpp>

using namespace std;

using database::DB;
using datetime::QuarterHour;

namespace weather {

HistoryQuarterHourHT::HistoryQuarterHourHT(unsigned int device_id)
:History(0), device_id(device_id)
{
	DB db;

	QuarterHour period_ago = QuarterHour() - retention;
	auto res = db.Query(" \
		SELECT log_ht_date, log_ht_min_h, log_ht_max_h, log_ht_min_t, log_ht_max_t \
		FROM t_log_ht  \
		WHERE device_id=%i \
		AND log_ht_date>=%s"_sql
		<<device_id<<string(period_ago)
	);

	while(res.FetchRow())
	{
		stat::MinMax<double> h(res["log_ht_min_h"], res["log_ht_max_h"]);
		stat::MinMax<double> t(res["log_ht_min_t"], res["log_ht_max_t"]);
		history.insert(pair<QuarterHour, MinMaxHT>(QuarterHour(res["log_ht_date"]), MinMaxHT(h, t)));
	}

	::thread::HistorySync::GetInstance()->Register(this);
}

HistoryQuarterHourHT::~HistoryQuarterHourHT()
{
	::thread::HistorySync::GetInstance()->Unregister(this);

	save();
}

void HistoryQuarterHourHT::store_entry(const QuarterHour period, MinMaxHT value)
{
	database::DB db;

	auto h = value.GetHumidity();
	auto t = value.GetTemperature();

	db.Query(" \
		REPLACE INTO t_log_ht(log_ht_date, device_id, log_ht_min_h, log_ht_max_h, log_ht_min_t, log_ht_max_t) \
		VALUES(%s, %i, %f, %f, %f, %f)"_sql
		<<string(period)<<device_id<<h.GetMin()<<h.GetMax()<<t.GetMin()<<t.GetMax()
	);
}

void HistoryQuarterHourHT::save()
{
	for(auto it=history.begin(); it!=history.end(); ++it)
		store_entry(it->first, it->second);
}

}

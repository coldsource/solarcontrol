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

#include <ht/HistoryQuarterHour.hpp>
#include <database/DB.hpp>
#include <thread/HistorySync.hpp>

#include <limits.h>

namespace ht {

using namespace std;

using database::DB;
using datetime::QuarterHour;
using ht::MinMax;

HistoryQuarterHour::HistoryQuarterHour(unsigned int device_id)
:History(0), device_id(device_id)
{
	DB db;

	QuarterHour period_ago = QuarterHour() - retention;
	auto res = db.Query(" \
		SELECT log_ht_date, log_ht_min_h, log_ht_max_h, log_ht_min_t, log_ht_max_t, log_ht_min_w, log_ht_max_w \
		FROM t_log_htw  \
		WHERE device_id=%i \
		AND log_ht_date>=%s"_sql
		<<device_id<<string(period_ago)
	);

	double invalid = std::numeric_limits<double>::quiet_NaN();

	while(res.FetchRow())
	{
		stat::MinMax<double> h(res["log_ht_min_h"].IsNull()?invalid:res["log_ht_min_h"], res["log_ht_max_h"].IsNull()?invalid:res["log_ht_max_h"]);
		stat::MinMax<double> t(res["log_ht_min_t"].IsNull()?invalid:res["log_ht_min_t"], res["log_ht_max_t"].IsNull()?invalid:res["log_ht_max_t"]);
		stat::MinMax<double> w(res["log_ht_min_w"].IsNull()?invalid:res["log_ht_min_w"], res["log_ht_max_w"].IsNull()?invalid:res["log_ht_max_w"]);
		history.insert(pair<QuarterHour, MinMax>(QuarterHour(res["log_ht_date"]),MinMax(h, t, w)));
	}

	::thread::HistorySync::GetInstance()->Register(this);
}
HistoryQuarterHour::~HistoryQuarterHour()
{
	::thread::HistorySync::GetInstance()->Unregister(this);

	save();
}

void HistoryQuarterHour::store_entry(const QuarterHour period, ht::MinMax value)
{
	database::DB db;

	auto h = value.GetHumidity();
	auto t = value.GetTemperature();
	auto w = value.GetWind();

	db.Query(" \
		REPLACE INTO t_log_htw(log_ht_date, device_id, log_ht_min_h, log_ht_max_h, log_ht_min_t, log_ht_max_t, log_ht_min_w, log_ht_max_w) \
		VALUES(%s, %i, %f, %f, %f, %f, %f, %f)"_sql
		<<string(period)<<device_id<<h.GetMin()<<h.GetMax()<<t.GetMin()<<t.GetMax()<<w.GetMin()<<w.GetMax()
	);
}

void HistoryQuarterHour::save()
{
	for(auto it=history.begin(); it!=history.end(); ++it)
		store_entry(it->first, it->second);
}

}

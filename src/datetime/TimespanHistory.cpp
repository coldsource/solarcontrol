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

#include <datetime/TimespanHistory.hpp>
#include <datetime/DateTimeRange.hpp>
#include <datetime/DateTime.hpp>
#include <datetime/Date.hpp>
#include <datetime/Timestamp.hpp>
#include <database/DB.hpp>
#include <configuration/ConfigurationSolarControl.hpp>

#include <stdexcept>

using namespace std;

namespace datetime
{

TimespanHistory::TimespanHistory(unsigned int device_id)
{
	retention_days = configuration::ConfigurationSolarControl::GetInstance()->GetInt("core.history.maxdays");

	if(device_id!=0)
	{
		// Reload database on/off history if device_id is provided
		database::DB db;
		Date days_ago = Date() - retention_days;
		auto res = db.Query(
			"SELECT log_state_date, log_state FROM t_log_state  WHERE device_id=%i AND log_state IS NOT NULL AND log_state_date>=%s ORDER BY log_state_date"_sql
			<<device_id<<string(days_ago)
		);

		while(res.FetchRow())
		{
			try
			{
				Timestamp ts = DateTime(res["log_state_date"]);
				int state = res["log_state"];
				if(state)
					ClockIn(ts);
				else
					ClockOut(ts);
			}
			catch(...) {} // Ignore clickin/clockout matching error cause logs retention can break cycles
		}
	}
}


void TimespanHistory::purge()
{
	Timestamp now(TS_REAL);
	auto it = history.begin();
	while(it!=history.end())
	{
		Timestamp end(it->to);
		if(now-end>retention_days*86400)
			it = history.erase(it);
		else
			++it;
	}
}

void TimespanHistory::ClockIn()
{
	ClockIn(Timestamp(TS_REAL));
}

void TimespanHistory::ClockIn(const Timestamp &ts)
{
	time_t from_t = ts;

	if(history.size()>0)
	{
		if(history.back().to==0)
			throw logic_error("Can't clock in, last period is not closed");

		if(history.back().to>from_t)
			throw logic_error("Timespans may not overlap");
	}

	history.push_back({from_t, 0});
}

void TimespanHistory::ClockOut()
{
	ClockOut(Timestamp(TS_REAL));
}

void TimespanHistory::ClockOut(const Timestamp &ts)
{
	time_t to_t = ts;

	if(history.size()==0 || history.back().to!=0)
		throw logic_error("Can't clock out, last period is already closed");

	if(history.back().from>to_t)
		throw logic_error("Can't clock out before click in time");

	history.back().to = to_t;

	purge();
}

unsigned long TimespanHistory::GetTotalForLast(int nseconds) const
{
	if(nseconds>retention_days * 86400)
		throw invalid_argument("History lookup can't be greater than retention days");

	time_t now = Timestamp(TS_REAL);
	time_t from_lookup_t = now - nseconds;
	unsigned long long total = 0;
	for(auto timespan : history)
	{
		Timestamp from;
		Timestamp to;

		if(timespan.to!=0 && timespan.to<from_lookup_t)
			continue; // Too far in the past

		if(timespan.from<from_lookup_t)
			from = from_lookup_t; // Period is split by our start lookup time
		else
			from = timespan.from; // Period is fully included

		if(timespan.to==0)
			to = now;
		else
			to = timespan.to;

		DateTimeRange range(from, to);
		Timestamp last_ts;
		for(Timestamp ts : range)
		{
			if(last_ts.Isempty())
			{
				last_ts = ts;
				continue;
			}

			total += ts-last_ts;

			DateTime cur_day(last_ts);
			cur_day.ToNoon();

			last_ts = ts;
		}
	}

	return total;
}

}

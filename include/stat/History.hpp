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

#ifndef __STAT_HISTORY_HPP__
#define __STAT_HISTORY_HPP__

#include <datetime/Timestamp.hpp>
#include <configuration/ConfigurationSolarControl.hpp>

#include <string>
#include <map>

namespace stat {

template<typename Period, typename DataType>
class History
{
	protected:
		std::map<Period, DataType> history;
		int retention;

		int sync_interval;
		datetime::Timestamp last_save_ts;

		virtual void store_entry(const Period period, DataType value) = 0;
		virtual void save() = 0;

		void purge()
		{
			Period period_ago = Period() - retention;

			auto it = history.begin();
			while(it!=history.end())
			{
				if(it->first<period_ago)
				{
					store_entry(it->first, it->second);
					it = history.erase(it);
				}
				else
					++it;
			}
		}

		void sync()
		{
			datetime::Timestamp now(TS_MONOTONIC);
			if(now - last_save_ts < sync_interval)
				return;

			save();

			last_save_ts = now;
		}

	public:
		History(int retention):retention(retention)
		{
			sync_interval = configuration::ConfigurationSolarControl::GetInstance()->GetTime("core.history.sync");
			last_save_ts = datetime::Timestamp(TS_MONOTONIC);
		}

		History(const History &h) = delete;

		void Set(DataType val)
		{
			Period now;
			history[now] = val;

			sync();
		}

		void Add(DataType val)
		{
			Period now;
			if(!history.contains(now))
				history[now] = val;
			else
				history[now] += val;

			purge();
			sync();
		}

		DataType GetTotalForLast(int nperiods) const
		{
			Period start_date = Period() - nperiods;
			DataType sum = 0;

			for(auto it=history.begin(); it!=history.end(); ++it)
			{
				if(it->first<start_date)
					continue;

				sum += it->second;
			}

			return sum;
		}

		DataType GetTotalForCurrent() const
		{
			Period now;

			auto it = history.find(now);
			if(it==history.end())
				return 0;

			return it->second;
		}

		const std::map<Period, DataType> &Get() const { return history; }
};

}

#endif

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

#ifndef __ENERGY_HISTORY_HPP__
#define __ENERGY_HISTORY_HPP__

#include <string>
#include <map>

namespace energy {

template<typename Period>
class History
{
	protected:
		std::string type;
		std::map<Period, double> history;
		int retention;

		virtual void store_entry(const Period period, double value) = 0;

		void purge()
		{
			Period period_ago = Period() - retention;

			auto it = history.begin();
			while(it!=history.end())
			{
				if(it->first<period_ago)
				{
					if(type!="")
						store_entry(it->first, it->second);

					it = history.erase(it);
				}
				else
					++it;
			}
		}

	public:
		History(int retention, const std::string &type = ""): type(type), retention(retention) {}

		History(const History &h) = delete;

		void Set(double energy)
		{
			Period now;
			history[now] = energy;
		}

		void Add(double energy)
		{
			Period now;
			if(!history.contains(now))
				history[now] = energy;
			else
				history[now] += energy;

			purge();
		}

		double GetTotalForLast(int nperiods) const
		{
			Period start_date = Period() - nperiods;
			double sum = 0;

			for(auto it=history.begin(); it!=history.end(); ++it)
			{
				if(it->first<start_date)
					continue;

				sum += it->second;
			}

			return sum;
		}

		double GetTotalForCurrent() const
		{
			Period now;

			auto it = history.find(now);
			if(it==history.end())
				return 0;

			return it->second;
		}

		const std::map<Period, double> &Get() const { return history; }
};

}

#endif

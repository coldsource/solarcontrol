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

#ifndef __UPGRADE_UPDRADE_HPP__
#define __UPGRADE_UPDRADE_HPP__

#include <string>
#include <memory>

namespace upgrade {

class Upgrade
{
	protected:
		std::string get_version() const;

	public:
		Upgrade() {}
		virtual ~Upgrade() {}

		static int VersionCompare(const std::string &v1, const std::string &v2);
		static bool CompareTo(std::shared_ptr<Upgrade> a, std::shared_ptr<Upgrade> b)
		{
			return (VersionCompare(a->target_version(), b->target_version()) < 0);
		}

		virtual std::string target_version() = 0;
		virtual bool is_needed() = 0;
		virtual void run() = 0;
};

}

#endif




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

#include <upgrade/Upgrades.hpp>
#include <upgrade/Upgrade_1_12.hpp>

#include <set>
#include <memory>

using namespace std;

namespace upgrade {

bool Upgrades::run()
{
	set<shared_ptr<Upgrade>, decltype(Upgrade::CompareTo) *> upgrades;
	upgrades.insert(make_shared<Upgrade_1_12>());

	int performed_upgrades = 0;
	for(auto upgrade : upgrades)
	{
		if(!upgrade->is_needed())
			continue;

		upgrade->run();
		performed_upgrades++;
	}

	return (performed_upgrades > 0);
}

}



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

#include <control/Plug.hpp>
#include <nlohmann/json.hpp>

using nlohmann::json;

namespace control {

Plug::Plug(const std::string &ip): HTTP(ip)
{
	state = get_output();
}

void Plug::Switch(bool new_state)
{
	json j;
	j["id"] = 1;
	j["method"] = "Switch.Set";
	j["params"]["id"] = 0;
	j["params"]["on"] = new_state;

	Post(j);

	state = new_state;
}

bool Plug::get_output() const
{
	json j;
	j["id"] = 1;
	j["method"] = "Switch.GetStatus";
	j["params"]["id"] = 0;

	auto out = Post(j);

	return out["result"]["output"];
}

void Plug::UpdateState()
{
	state = get_output();
}

}

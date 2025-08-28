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

#ifndef  __ENERGY_CONFIGURATIONENERGY_H__
#define  __ENERGY_CONFIGURATIONENERGY_H__

#include <string>

#include <configuration/ConfigurationPart.hpp>

namespace configuration
{

class ConfigurationEnergy:public ConfigurationPart
{
	public:
		ConfigurationEnergy(void);
		virtual ~ConfigurationEnergy(void);

		std::string GetType() const override { return "energy"; }

		void Check(void) const override;
};

}

#endif


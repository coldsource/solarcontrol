/*
 * This file is part of evQueue
 *
 * evQueue is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * evQueue is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with evQueue. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Thibault Kummer <bob@coldsource.net>
 */

#ifndef  __CONFIGURATION_CONFIGURATIONSOLARCONTROL_H__
#define  __CONFIGURATION_CONFIGURATIONSOLARCONTROL_H__

#include <string>
#include <map>

#include <configuration/Configuration.hpp>

namespace configuration
{

class ConfigurationSolarControl:public Configuration
{
	public:
		ConfigurationSolarControl(void);
		virtual ~ConfigurationSolarControl(void);

		static ConfigurationSolarControl *GetInstance() { return (ConfigurationSolarControl *)instance; }

		void Check(void);
};

}

#endif


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

#include <utils/config.hpp>
#include <configuration/Configuration.hpp>
#include <configuration/ConfigurationReader.hpp>
#include <logs/Logger.hpp>
#include <excpt/Config.hpp>
#include <excpt/ConfigParser.hpp>

#include <stdio.h>

using namespace std;

namespace utils {

configuration::Configuration *load_config(const std::string &filename)
{
	// Read and check configuration from file
	logs::Logger::Log(LOG_INFO, "Reading configuration file");
	auto config = configuration::Configuration::GetInstance();

	try
	{
		configuration::ConfigurationReader::Read(filename, config);
		config->Check();

		// Backup this version of configuration as master configuration. This is used as a default configuration setup
		config->Backup("master");

		return config;
	}
	catch(excpt::ConfigParser &e)
	{
		e.Log(LOG_ERR);
	}
	catch(excpt::Config &e)
	{
		e.Log(LOG_ERR);
	}

	exit(-1); // Configuration error
}

}


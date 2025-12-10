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

#include <utils/args.hpp>
#include <configuration/Args.hpp>
#include <logs/Logger.hpp>
#include <excpt/Config.hpp>

#include <stdio.h>

using namespace std;

namespace utils {

void print_usage()
{
	fprintf(stderr,"Usage :\n");
	fprintf(stderr,"  Launch Solar Control : solarcontrol --config <path to config file>\n");
	fprintf(stderr,"  Show version         : solarcontrol --version\n");
	fprintf(stderr,"  Perform upgrade      : solarcontrol --config <path to config file> --upgrade\n");
}

configuration::Args check_args(int argc, char **argv)
{
	const map<string, string> args_config = {
		{"--config", "string"},
		{"--version", "flag"},
		{"--upgrade", "flag"}
	};

	try
	{
		configuration::Args args(args_config, argc, argv);

		if(args["--version"])
		{
			printf(VERSION "\n");
			exit(0);
		}

		string config_filename = args["--config"];
		if(config_filename=="")
		{
			utils::print_usage();
			exit(-3);
		}

		return args;
	}
	catch(excpt::Config &e)
	{
		e.Log(LOG_ERR);
		utils::print_usage();
		exit(-2);
	}
}

}

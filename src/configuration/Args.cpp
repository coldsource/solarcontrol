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

#include <configuration/Args.hpp>
#include <excpt/Config.hpp>

#include <stdio.h>
#include <unistd.h>

using namespace std;

namespace configuration
{

Args::Args(const map<string, string> &config, int argc, char **argv)
{
	for(auto it = config.begin(); it!=config.end(); ++it)
	{
		if(it->second=="flag")
		{
			vals[it->first] = "no";
			types[it->first] = "flag";
			required[it->first] = false;
		}
		else if(it->second=="string")
		{
			vals[it->first] = "";
			types[it->first] = "string";
			required[it->first] = false;
		}
		else if(it->second=="rstring")
		{
			vals[it->first] = "";
			types[it->first] = "string";
			required[it->first] = true;
		}
		else if(it->second=="integer")
		{
			vals[it->first] = "-1";
			types[it->first] = "integer";
			required[it->first] = false;
		}
		else if(it->second=="rinteger")
		{
			vals[it->first] = "";
			types[it->first] = "integer";
			required[it->first] = true;
		}
		else
			throw invalid_argument("Invalid configuration. Unknown type : "+it->second);
	}

	for(int i=1;i<argc;i++)
	{
		string arg(argv[i]);

		if(config.find(arg)==config.end())
			throw excpt::Config("Unknown argument "+arg, arg);

		if(types[arg]=="flag")
			vals[arg] = "yes";
		else if(types[arg]=="string" || types[arg]=="integer")
		{
			if(i+1>=argc)
				throw excpt::Config("Missing value for argument "+arg, arg);

			string val(argv[i+1]);

			if(types[arg]=="integer")
			{
				try
				{
					stoi(val);
				}
				catch(...)
				{
					throw excpt::Config("Invalid integer value for argument "+arg, arg);
				}
			}

			vals[arg] = val;
			i++;
		}
	}

	for(auto it = vals.begin(); it!=vals.end(); ++it)
	{
		if(required[it->first] && vals[it->first]=="")
			throw excpt::Config("Missing required argument : "+it->first, it->first);
	}
}

Args::args_val Args::operator[](const string &name)
{
	if(vals.find(name)==vals.end())
		throw excpt::Config("Unknown argument "+name, name);

	return args_val(name, vals[name], types[name]);
}

Args::args_val::args_val(const string &name, const string val, const string type)
{
	this->name = name;
	this->val = val;
	this->type = type;
}

Args::args_val::operator bool() const
{
	if(type!="flag")
		throw excpt::Config("Argument "+name+" is not a flag", name);

	return val=="yes";
}

Args::args_val::operator int() const
{
	if(type!="integer")
		throw excpt::Config("Argument "+name+" is not an integer", name);

	return val==""?0:stoi(val);
}

Args::args_val::operator std::string() const
{
	if(type!="string")
		throw excpt::Config("Argument "+name+" is not an string", name);

	return val;
}

}

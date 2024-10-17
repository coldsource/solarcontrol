#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <signal.h>

#include <datetime/DateTime.hpp>
#include <mqtt/Client.hpp>
#include <datetime/Timestamp.hpp>
#include <datetime/HourMinuteSecond.hpp>
#include <datetime/DateTimeRange.hpp>
#include <datetime/TimespanHistory.hpp>
#include <energy/Counter.hpp>
#include <energy/GlobalMeter.hpp>
#include <control/Plug.hpp>
#include <device/Device.hpp>
#include <device/Devices.hpp>
#include <device/DeviceTimeRange.hpp>
#include <thread/DevicesManager.hpp>
#include <utils/signal.hpp>
#include <configuration/Args.hpp>
#include <configuration/Configuration.hpp>
#include <configuration/ConfigurationReader.hpp>
#include <configuration/ConfigurationSolarControl.hpp>
#include <database/DB.hpp>
#include <database/Query.hpp>
#include <websocket/SolarControl.hpp>
#include <thread/LCD.hpp>

#include <nlohmann/json.hpp>

#include <vector>
#include <set>
#include <map>

using nlohmann::json;
using namespace std;

void signal_callback_handler(int signum)
{
	if(signum==SIGINT || signum==SIGTERM)
	{
		::thread::DevicesManager::GetInstance()->Shutdown();
	}
	else if(signum==SIGHUP)
	{
		device::Devices::GetInstance()->Reload();
	}
}

void tools_print_usage()
{
	fprintf(stderr,"Usage :\n");
	fprintf(stderr,"  Launch Solar Control : solarcontrol --config <path to config file>\n");
	fprintf(stderr,"  Show version         : solarcontrol --version\n");
}


int main(int argc, char **argv)
{
	const map<string, string> args_config = {
		{"--config", "string"},
		{"--version", "flag"}
	};

	configuration::Args args;
	try
	{
		args = configuration::Args(args_config, argc, argv);
	}
	catch(exception &e)
	{
		tools_print_usage();
		return 0;
	}

	if(args["--version"])
	{
		printf("SolarControl version 0.1");
		return 0;
	}

	string config_filename = args["--config"];
	if(config_filename=="")
	{
		tools_print_usage();
		return -1;
	}

	mosquitto_lib_init();
	curl_global_init(CURL_GLOBAL_ALL);
	database::DB::InitLibrary();

	// Position signal handlers
	utils::set_sighandler(signal_callback_handler, {SIGINT, SIGTERM, SIGHUP});

	// Read configuration
	configuration::Configuration *config = configuration::Configuration::GetInstance();
	config->Merge();

	configuration::ConfigurationReader::Read(config_filename, config);
	config->Split();
	config->CheckAll();

	auto config_sc = configuration::ConfigurationSolarControl::GetInstance();
	mqtt::Client mqtt(config_sc->Get("mqtt.host"), config_sc->GetInt("mqtt.port"));

	energy::GlobalMeter globalmeter;

	websocket::SolarControl ws;
	ws.Start();

	::thread::LCD lcd;

	::thread::DevicesManager dev_manager;
	dev_manager.WaitForShutdown();

	mqtt.Shutdown();
	mqtt.WaitForShutdown();

	ws.Shutdown();

	lcd.Shutdown();
	lcd.WaitForShutdown();

	delete config;

	mosquitto_lib_cleanup();
	curl_global_cleanup();
	database::DB::FreeLibrary();

	return 0;
}

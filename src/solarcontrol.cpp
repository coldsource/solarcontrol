#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <signal.h>

#include <mqtt/Client.hpp>
#include <energy/GlobalMeter.hpp>
#include <device/Devices.hpp>
#include <device/electrical/DeviceHWS.hpp>
#include <device/electrical/DeviceGrid.hpp>
#include <device/electrical/DevicePV.hpp>
#include <device/electrical/DeviceBattery.hpp>
#include <utils/signal.hpp>
#include <configuration/Args.hpp>
#include <configuration/Configuration.hpp>
#include <configuration/ConfigurationReader.hpp>
#include <configuration/ConfigurationReaderDB.hpp>
#include <configuration/ConfigurationSolarControl.hpp>
#include <database/DB.hpp>
#include <database/DBConfig.hpp>
#include <websocket/SolarControl.hpp>
#include <thread/DevicesManager.hpp>
#include <thread/LCD.hpp>
#include <thread/HistorySync.hpp>

#include <nlohmann/json.hpp>

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
		device::Devices().Reload();
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
		return -2;
	}

	if(args["--version"])
	{
		printf(VERSION "\n");
		return 0;
	}

	string config_filename = args["--config"];
	if(config_filename=="")
	{
		tools_print_usage();
		return -3;
	}

	configuration::Configuration *config = 0;
	short exit_code = 0;

	mosquitto_lib_init();
	curl_global_init(CURL_GLOBAL_ALL);
	database::DB::InitLibrary();

	try
	{
		// Position signal handlers
		utils::set_sighandler(signal_callback_handler, {SIGINT, SIGTERM, SIGHUP});

		// Read and check configuration from file
		config = configuration::Configuration::GetInstance();
		configuration::ConfigurationReader::Read(config_filename, config);
		config->Check();

		// Backup this version of configuration as master configuration. This is used as a default configuration setup
		config->Backup("master");

		// Init database tables
		auto dbconfig = database::DBConfig::GetInstance();
		dbconfig->InitTables();

		configuration::ConfigurationReaderDB::Read(config);

		// Create history sync thread before devices
		::thread::HistorySync histo_sync;

		auto config_sc = configuration::Configuration::FromType("solarcontrol");
		mqtt::Client mqtt(config_sc->Get("mqtt.host"), config_sc->GetInt("mqtt.port"));

		// Create special devices if needed
		device::DeviceHWS::CreateInDB();
		device::DeviceGrid::CreateInDB();
		device::DevicePV::CreateInDB();
		device::DeviceBattery::CreateInDB();

		device::Devices devices;

		// Create global energy meter (grid, pv, hws)
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

		devices.Unload(); // Unload all devices to ensure MQTT Unsubscription before destructor is called on MQTT client
	}
	catch(exception &e)
	{
		fprintf(stderr, "%s\n", e.what());
		exit_code = 1;
	}

	mosquitto_lib_cleanup();
	curl_global_cleanup();
	database::DB::FreeLibrary();

	return exit_code;
}

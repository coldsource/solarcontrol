#include <stdio.h>
#include <unistd.h>
#include <curl/curl.h>
#include <signal.h>

#include <mqtt/Client.hpp>
#include <energy/GlobalMeter.hpp>
#include <device/Devices.hpp>
#include <device/electrical/HWS.hpp>
#include <device/electrical/Grid.hpp>
#include <device/electrical/PV.hpp>
#include <device/electrical/Battery.hpp>
#include <utils/signal.hpp>
#include <utils/args.hpp>
#include <utils/config.hpp>
#include <configuration/Args.hpp>
#include <configuration/Configuration.hpp>
#include <configuration/ConfigurationReader.hpp>
#include <configuration/ConfigurationReaderDB.hpp>
#include <configuration/ConfigurationSolarControl.hpp>
#include <database/DB.hpp>
#include <database/DBConfig.hpp>
#include <websocket/SolarControl.hpp>
#include <thread/DevicesManager.hpp>
#include <thread/Stats.hpp>
#include <thread/SensorsManager.hpp>
#include <thread/LCD.hpp>
#include <thread/HistorySync.hpp>
#include <logs/Logger.hpp>
#include <excpt/Config.hpp>
#include <excpt/ConfigParser.hpp>
#include <upgrade/Upgrades.hpp>

#include <nlohmann/json.hpp>

#include <map>

using nlohmann::json;
using namespace std;

bool startup_finished = false;

int main(int argc, char **argv)
{
	configuration::Args args = utils::check_args(argc, argv);

	short exit_code = 0;

	mosquitto_lib_init();
	curl_global_init(CURL_GLOBAL_ALL);
	database::DB::InitLibrary();

	try
	{
		// Position signal handlers
		utils::set_sighandler(utils::signal_callback_handler, {SIGINT, SIGTERM, SIGHUP});

		auto config = utils::load_config(args["--config"]);

		// Init database tables
		auto dbconfig = database::DBConfig::GetInstance();
		dbconfig->InitTables();

		configuration::ConfigurationReaderDB::Read(config);

		if(args["--upgrade"])
		{
			// Run any necessary version upgrades
			upgrade::Upgrades upgrades;
			bool did_upgrade = upgrades.run();

			if(did_upgrade)
				logs::Logger::Log(LOG_INFO, "Upgrades sucessfully applied");
			else
				logs::Logger::Log(LOG_INFO, "No upgrades to apply");

			return 0;
		}

		// Create history sync thread before devices
		::thread::HistorySync histo_sync;

		// Create MQTT client before devices so they can register to it
		auto config_sc = configuration::Configuration::FromType("solarcontrol");
		mqtt::Client mqtt(config_sc->Get("mqtt.host"), config_sc->GetInt("mqtt.port"));

		// Create special devices if needed
		device::HWS::CreateInDB();
		device::Grid::CreateInDB();
		device::PV::CreateInDB();
		device::Battery::CreateInDB();

		// Create sensors manager before devices so they can register to it
		::thread::SensorsManager sensors_manager;

		logs::Logger::Log(LOG_INFO, "Loading devices");
		device::Devices devices;

		// Create global energy meter (grid, pv, hws, battery)
		energy::GlobalMeter globalmeter;

		logs::Logger::Log(LOG_INFO, "Starting Websocket server");
		websocket::SolarControl ws;
		ws.Start();

		::thread::LCD lcd;

		// Once all is setup, start devices related threads
		::thread::Stats stats;
		::thread::DevicesManager dev_manager;
		stats.Start();
		sensors_manager.Start();
		dev_manager.Start();
		mqtt.Start();

		// Flag startup sequence as completed, from now we can trigger exit from signals
		startup_finished = true;

		stats.WaitForShutdown();
		sensors_manager.WaitForShutdown();
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
		logs::Logger::Log(LOG_ERR, "Unexpected exception : " + string(e.what()));
		exit_code = 1;
	}

	mosquitto_lib_cleanup();
	curl_global_cleanup();
	database::DB::FreeLibrary();

	return exit_code;
}

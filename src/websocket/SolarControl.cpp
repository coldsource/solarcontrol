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

#include <websocket/SolarControl.hpp>
#include <database/DB.hpp>
#include <energy/GlobalMeter.hpp>
#include <device/Devices.hpp>
#include <device/electrical/DeviceElectrical.hpp>
#include <device/weather/DeviceWeather.hpp>
#include <logs/Logger.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

namespace websocket
{

SolarControl *SolarControl::instance = 0;

SolarControl::SolarControl(): Server()
{
	instance = this;
}

SolarControl::~SolarControl()
{

}

void SolarControl::NotifyAll(unsigned int protocol)
{
	unique_lock<recursive_mutex> llock(lock);

	for(auto client : clients[protocol])
		CallbackOnWritable(client);

	cancel_service();
}

map<string, unsigned int> SolarControl::get_protocols()
{
	map<string, unsigned int> protocols;
	protocols["api"] = en_protocols::API;
	protocols["meter"] = en_protocols::METER;
	protocols["device"] = en_protocols::DEVICE;
	return protocols;
}

void SolarControl::context_creation(struct lws_context_creation_info *info)
{
	info->server_string = "SolarControl websockets server";
}

void SolarControl::start_thread(void)
{
	database::DB::StartThread();
}

void SolarControl::stop_thread(void)
{
	database::DB::StopThread();
}

void SolarControl::worker(struct lws *wsi, st_api_context *api_ctx)
{
	database::DB::StartThread();

	try
	{
		string message = api_ctx->message;
		string response = "";

		if(message!="")
			response = instance->dispatcher.Dispatch(message); // Callback with no message, skip

		api_ctx->message = ""; // API command is answered, remove question from context
		api_ctx->response = response; // Set response
	}
	catch(exception &e)
	{
		logs::Logger::Log(LOG_ERR, e.what());
	}

	database::DB::StopThread();

	unique_lock<recursive_mutex> llock(instance->lock);
	if(instance->IsCancelling())
		return; // Shutdown in progress, we are waited and must be joined

	if(api_ctx->is_orphaned)
	{
		// We are an orphaned worker, detach thread and remove our context
		api_ctx->worker.detach();
		delete api_ctx;
		return;
	}

	instance->CallbackOnWritable(wsi);
}

void *SolarControl::lws_callback_established(struct lws *wsi, unsigned int protocol)
{
	unique_lock<recursive_mutex> llock(lock);

	clients[protocol].insert(wsi);

	if(protocol==DEVICE || protocol==METER)
		NotifyAll(protocol);

	if(protocol==API)
		return new st_api_context();

	return 0;
}

void SolarControl::lws_callback_closed(struct lws *wsi, unsigned int protocol, void *user_data)
{
	unique_lock<recursive_mutex> llock(lock);

	if(protocol==API)
	{
		st_api_context *api_ctx = (st_api_context *)user_data;

		if(api_ctx->worker_alive)
		{
			if(IsCancelling())
			{
				llock.unlock();
				api_ctx->worker.join(); // Shutting down, wait thread
				llock.lock();

				delete api_ctx;
			}
			else
				api_ctx->is_orphaned = true; // Interrupted while worker is still alive, require future deletition
		}
		else
			delete api_ctx;
	}

	clients[protocol].erase(wsi);
}

void SolarControl::lws_callback_receive(struct lws *wsi, unsigned int protocol, const std::string &message, void *user_data)
{
	if(protocol==API)
	{
		unique_lock<recursive_mutex> llock(lock);
		st_api_context *api_ctx = (st_api_context *)user_data;

		if(api_ctx->worker_alive)
			return; // Worker still alive, next command must be sent after

		api_ctx->message = message;
		api_ctx->worker_alive = true;
		api_ctx->worker = thread(worker, wsi, api_ctx);
	}
}

std::string SolarControl::lws_callback_server_writeable(struct lws * /* wsi */, unsigned int protocol, void *user_data)
{
	if(protocol==API)
	{
		st_api_context *api_ctx = (st_api_context *)user_data;
		if(!api_ctx->worker_alive)
			return ""; // Spurious call
		api_ctx->worker.join();
		api_ctx->worker_alive = false;
		return api_ctx->response;
	}
	else if(protocol==METER)
	{
		auto global = energy::GlobalMeter::GetInstance();

		json j;
		j["grid"] = global->GetGridPower();
		j["pv"] = global->GetPVPower();
		j["hws"] = global->GetHWSPower();

		j["total"] = global->GetPower();
		j["net_available"] = global->GetNetAvailablePower();
		j["gross_available"] = global->GetGrossAvailablePower();
		j["excess"] = global->GetExcessPower();

		j["grid_energy"] = global->GetGridEnergy();
		j["grid_exported_energy"] = global->GetExportedEnergy();
		j["pv_energy"] = global->GetPVEnergy();
		j["hws_energy"] = global->GetHWSEnergy();
		j["hws_energy_offload"] = global->GetHWSOffloadEnergy();

		j["offpeak"] = global->GetOffPeak();

		return string(j.dump());
	}
	else if(protocol==DEVICE)
	{
		json j_devices = json::array();

		device::Devices devices;

		auto devices_electrical = devices.GetElectrical();
		for(auto device : devices_electrical)
		{
			json j_device;
			j_device["device_id"] = device->GetID();
			j_device["device_type"] = device->GetType();
			j_device["device_name"] = device->GetName();
			j_device["device_config"] = (json)device->GetConfig();
			j_device["state"] = device->GetState();
			j_device["manual"] = device->IsManual();
			j_device["power"] = device->GetPower();

			j_devices.push_back(j_device);
		}

		auto devices_weather = devices.GetWeather();
		for(auto device : devices_weather)
		{
			json j_device;

			j_device["device_id"] = device->GetID();
			j_device["device_type"] = device->GetType();
			j_device["device_name"] = device->GetName();
			j_device["device_config"] = (json)device->GetConfig();
			j_device["temperature"] = device->GetTemperature();
			j_device["humidity"] = device->GetHumidity();
			j_device["wind"] = device->GetWind();

			j_devices.push_back(j_device);
		}

		return string(j_devices.dump());
	}

	return "";
}

}

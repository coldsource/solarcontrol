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
	unique_lock<mutex> llock(lock);

	for(auto client : clients[protocol])
		lws_callback_on_writable(client);

	cancel_service();
}

map<string, unsigned int> SolarControl::get_protocols()
{
	map<string, unsigned int> protocols;
	protocols["api"] = en_protocols::API;
	protocols["meter"] = en_protocols::METER;
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

void *SolarControl::lws_callback_established(struct lws *wsi, unsigned int protocol)
{
	unique_lock<mutex> llock(lock);

	clients[protocol].insert(wsi);

	return new st_api_context();;
}

void SolarControl::lws_callback_closed(struct lws *wsi, unsigned int protocol, void *user_data)
{
	unique_lock<mutex> llock(lock);

	clients[protocol].erase(wsi);
	delete (st_api_context *)user_data;
}

void SolarControl::lws_callback_receive(struct lws *wsi, unsigned int protocol, const std::string &message, void *user_data)
{
	if(protocol==API)
	{
		st_api_context *api_ctx = (st_api_context *)user_data;
		api_ctx->message = message;

		lws_callback_on_writable(wsi);
	}
}

std::string SolarControl::lws_callback_server_writeable(struct lws *wsi, unsigned int protocol, void *user_data)
{
	if(protocol==API)
	{
		st_api_context *api_ctx = (st_api_context *)user_data;

		return dispatcher.Dispatch(api_ctx->message);
	}
	else if(protocol==METER)
	{
		auto global = energy::GlobalMeter::GetInstance();

		json j;
		j["grid"] = global->GetGridPower();
		j["pv"] = global->GetPVPower();

		j["total"] = global->GetPower();
		j["net_available"] = global->GetNetAvailablePower();
		j["gross_available"] = global->GetGrossAvailablePower();
		j["excess"] = global->GetExcessPower();

		j["grid_energy"] = global->GetGridEnergy();
		j["grid_exported_energy"] = global->GetExportedEnergy();
		j["pv_energy"] = global->GetPVEnergy();
		j["hws_energy"] = global->GetHWSEnergy();

		return string(j.dump());
	}

	return "";
}

}

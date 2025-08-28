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

#ifndef __SOLARCONTROL_HPP__
#define __SOLARCONTROL_HPP__

#include <websocket/Server.hpp>
#include <api/Dispatcher.hpp>

#include <set>
#include <map>
#include <mutex>
#include <thread>

namespace websocket
{

class SolarControl: public Server
{
	static SolarControl *instance;
	std::recursive_mutex lock;

	std::map<unsigned int, std::set<struct lws *>> clients;

	struct st_api_context
	{
		std::string message = "";
		std::string response = "";
		bool worker_alive = false;
		std::thread worker;
		bool is_orphaned = false;
	};

	api::Dispatcher dispatcher;

	// API workers
	static void worker(struct lws *wsi, st_api_context *api_ctx);

	public:
		enum en_protocols
		{
			API,
			METER,
			DEVICE,
			CONFIG
		};

		SolarControl();
		~SolarControl();

		static SolarControl *GetInstance() { return instance; }

		void NotifyAll(unsigned int protocol);

	protected:
		std::map<std::string, unsigned int> get_protocols() override;
		void context_creation(struct lws_context_creation_info *info) override;

		void start_thread(void) override;
		void stop_thread(void) override;

		void *lws_callback_established(struct lws *wsi, unsigned int protocol) override;
		void lws_callback_closed(struct lws *wsi, unsigned int protocol, void *user_data) override;
		void lws_callback_receive(struct lws *wsi, unsigned int protocol, const std::string &message, void *user_data) override;
		std::string lws_callback_server_writeable(struct lws * /* wsi */, unsigned int protocol, void *user_data) override;
};

}

#endif

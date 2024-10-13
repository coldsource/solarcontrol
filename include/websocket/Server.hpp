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

#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <string>
#include <thread>
#include <mutex>
#include <set>
#include <map>
#include <vector>

#include <libwebsockets.h>

namespace websocket
{

class Server
{
	bool is_cancelling = false;

	std::map<std::string, unsigned int> user_def_protocols;
	std::vector<lws_protocols> protocols;

	struct lws_context_creation_info info;
	struct lws_context *context;

	std::thread ws_worker;

	public:
		struct per_session_data
		{
			std::string *cmd_buffer;
			void *derived_session_data;
		};

	public:
		Server();
		virtual ~Server();

		void Start();
		void Shutdown();

	public:
		static int callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len );
		static int callback_evq(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

		static void event_loop(Server *ws);

	protected:
		void cancel_service() { lws_cancel_service(context); }

		virtual std::map<std::string, unsigned int> get_protocols() { return std::map<std::string, unsigned int>(); }
		virtual void context_creation(struct lws_context_creation_info *info) {}

		void start_thread(void) {}
		void stop_thread(void) {}

		virtual void *lws_callback_established(struct lws *wsi, unsigned int protocol) { return 0; }
		virtual void lws_callback_closed(struct lws *wsi, unsigned int protocol, void *user_data) {}
		virtual void lws_callback_receive(struct lws *wsi, unsigned int protocol, const std::string &message, void *user_data) {}
		virtual std::string lws_callback_server_writeable(struct lws *wsi, unsigned int protocol, void *user_data) { return ""; }
};

}

#endif

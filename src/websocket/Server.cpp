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

#include <websocket/Server.hpp>

#include <stdexcept>

using namespace std;

namespace websocket
{

Server::Server()
{
}

void Server::Start()
{
	memset(&info, 0, sizeof(info));

	// Create protocols
	protocols.push_back({"http-only", Server::callback_http, 0, 0, 0, 0, 0}); // Dummy http protocol

	// Custom defined protocols
	user_def_protocols = get_protocols();
	for(auto protocol = user_def_protocols.begin(); protocol!=user_def_protocols.end(); ++protocol)
	{
		protocols.push_back({
			protocol->first.c_str(),
			Server::callback_ws,
			sizeof(Server::per_session_data),
			32768,
			protocol->second,
			0,
			0
		});
	}

	protocols.push_back({0,0,0,0,0,0,0}); // End of protocols


	lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE, [](int /* level */, const char * /* msg */) {
	});

	info.keepalive_timeout = 60;
	info.timeout_secs = 30;
	info.port = 7000;
	info.protocols = protocols.data();
	info.gid = (gid_t)-1;
	info.uid = (uid_t)-1;
	info.count_threads = 1;
	info.server_string = "libWebsockets Server";
	info.vhost_name = "default";
	info.user = this;

	context_creation(&info);

	context = lws_create_context(&info);
	if(!context)
		throw runtime_error("Unable to bind port "+to_string(info.port));

	ws_worker = thread(event_loop, this);
}

Server::~Server()
{
}

int Server::callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	uint8_t buf[LWS_PRE + 2048], *start = &buf[LWS_PRE], *p = start, *end = &buf[sizeof(buf) - 1];
	string text = "Forbidden";

	switch( reason )
	{
		case LWS_CALLBACK_HTTP:
			if (lws_add_http_common_headers(wsi, 403, "text/plain", (lws_filepos_t)text.length(), &p, end))
				return 1;

			if (lws_finalize_write_http_header(wsi, start, &p, end))
				return 1;

			lws_callback_on_writable(wsi);
			return 0;

		case LWS_CALLBACK_HTTP_WRITEABLE:
			text.insert(0,LWS_PRE,' ');
			if ((size_t)lws_write(wsi, (uint8_t *)text.c_str() + LWS_PRE, (unsigned int)text.length() - LWS_PRE, LWS_WRITE_HTTP_FINAL) != text.length() - LWS_PRE)
				return 1;

			if (lws_http_transaction_completed(wsi))
				return -1;

			return 0;

		default:
			return lws_callback_http_dummy(wsi, reason, user, in, len);
	}
}

int Server::callback_ws(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	struct lws_context *g_context = lws_get_context(wsi);
	Server *ws = (Server *)lws_context_user(g_context);


	per_session_data *context = (per_session_data *)user;
	const lws_protocols *protocol = lws_get_protocol(wsi);

	try
	{
		switch(reason)
		{
			case LWS_CALLBACK_PROTOCOL_INIT:
				break;

			case LWS_CALLBACK_PROTOCOL_DESTROY:
				break;

			case LWS_CALLBACK_ESTABLISHED:
			{
				// Store client
				ws->clients[protocol->id].insert(wsi);

				// Init context data
				context->cmd_buffer = new string();
				context->derived_session_data = ws->lws_callback_established(wsi, protocol->id);
				break;
			}

			case LWS_CALLBACK_CLOSED:
			{
				ws->lws_callback_closed(wsi, protocol->id, context->derived_session_data);

				// Remove client
				ws->clients[protocol->id].erase(wsi);

				{
					unique_lock<mutex> llock(ws->lock);
					ws->notified_clients.erase(wsi);
				}

				// Clean context data
				delete context->cmd_buffer;
				break;
			}

			case LWS_CALLBACK_RECEIVE:
			{
				// Message has been received
				string input_str((char *)in,len);

				// Handle multi packets messages
				if(lws_remaining_packet_payload(wsi)>0)
				{
					// Message is not complete, wait next packet
					*context->cmd_buffer += input_str;
					break;
				}
				else if(!context->cmd_buffer->empty())
				{
					// We are last part of a split message
					input_str = *context->cmd_buffer + input_str;
					context->cmd_buffer->clear();
				}

				ws->lws_callback_receive(wsi, protocol->id, input_str, context->derived_session_data);

				break;
			}

			case LWS_CALLBACK_SERVER_WRITEABLE:
			{
				string response = ws->lws_callback_server_writeable(wsi, protocol->id, context->derived_session_data);
				if(response=="")
					break;

				response.insert(0,LWS_PRE,' ');
				lws_write(wsi, (unsigned char *)response.c_str() + LWS_PRE, response.length()-LWS_PRE, LWS_WRITE_TEXT);

				break;
			}

			default:
				break;
		}
	}
	catch(exception &e)
	{
		lws_close_reason(wsi, LWS_CLOSE_STATUS_UNEXPECTED_CONDITION, (unsigned char *)e.what(), strlen(e.what()));
		return -1;
	}

	return 0;
}

void Server::notify_all(unsigned int protocol)
{
	{
		// This is not thread safe and must be done locked
		unique_lock<mutex> llock(lock);

		notified_protocols.insert(protocol);
	}

	if(this_thread::get_id()!=ws_worker.get_id())
		cancel_service(); // Thread safe according to LWS : Cancel main event loop to handle event
}

void Server::callback_on_writable(struct lws *wsi)
{
	{
		// This is not thread safe and must be done locked
		unique_lock<mutex> llock(lock);

		notified_clients.insert(wsi);
	}

	if(this_thread::get_id()!=ws_worker.get_id())
		cancel_service(); // Thread safe according to LWS : Cancel main event loop to handle event
}

void Server::Shutdown(void)
{
	is_cancelling = true;
	lws_cancel_service(context);

	ws_worker.join();

	lws_context_destroy(context);
}

void Server::event_loop(Server *ws)
{
	ws->start_thread();

	while( 1 )
	{
		lws_service(ws->context, 10000);

		if(ws->is_cancelling)
			break;

		// Check if we have protocol notifications requested
		set<unsigned int> protocols;
		set<struct lws *> clients;

		{
			// Atomically fetch the set of protocols and clients notified
			unique_lock<mutex> llock(ws->lock);

			protocols = ws->notified_protocols;
			ws->notified_protocols.clear();

			clients = ws->notified_clients;
			ws->notified_clients.clear();
		}

		// Handle local protocols list unlocked
		for(unsigned int protocol : protocols)
		{
			for(auto client : ws->clients[protocol])
				lws_callback_on_writable(client);
		}

		// Handle local clients list unlocked
		for(auto client : clients)
			lws_callback_on_writable(client);
	}

	ws->stop_thread();
}

}

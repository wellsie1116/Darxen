/* restapi.c
 *
 * Copyright (C) 2009 - Kevin Wells <kevin@darxen.org>
 *
 * This file is part of darxen
 *
 * darxen is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * darxen is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with darxen.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "restapi.h"

#include "ClientManager.h"
#include "Client.h"

#include <json-glib/json-glib.h>

#include <curl/curl.h> //FIXME: include something less
#include <stdarg.h>
#include <stdint.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PORT 4889
#define PAGE "<html><head><title>libmicrohttpd demo</title>"\
			 "</head><body>libmicrohttpd demo</body></html>"

#define PAGE_AUTHENTICATE "<html><body>Please authenticate</body></html>"
#define PAGE_FAIL "<html><body>This isn't the server you are looking for</body></html>"

static struct MHD_Daemon* d;

static inline int respond_authenticate(struct MHD_Connection* connection)
{
	struct MHD_Response* response;
	int ret;
	response = MHD_create_response_from_buffer(strlen(PAGE_AUTHENTICATE), PAGE_AUTHENTICATE, MHD_RESPMEM_PERSISTENT);
	ret =  MHD_queue_basic_auth_fail_response(connection, "darxen daemon", response);
	MHD_destroy_response(response);
	return ret;
}

static inline int respond_success(struct MHD_Connection* connection)
{
	struct MHD_Response* response;
	int ret;
	response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
	ret = MHD_queue_response(connection, MHD_HTTP_NO_CONTENT, response);
	MHD_destroy_response(response);
	return ret;
}

static inline int respond_fail(struct MHD_Connection* connection)
{
	struct MHD_Response* response;
	int ret;
	response = MHD_create_response_from_buffer(strlen(PAGE_FAIL), PAGE_FAIL, MHD_RESPMEM_PERSISTENT);
	ret =  MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
	MHD_destroy_response(response);
	return ret;
}

static int handle_request(  void* cls,
							struct MHD_Connection* connection,
							const char* url,
							const char* method,
							const char* version,
							const char* upload_data,
							size_t* upload_data_size,
							void** ptr)
{
	struct MHD_Response* response;
	int ret;

	DarxendClient* client = NULL;

	if (!*ptr) 
	{
		/* The first time only the headers are valid,
		 do not respond in the first round... */
		*ptr = connection;
		return MHD_YES;
	}

	char* pass = NULL;
	char* user = MHD_basic_auth_get_username_password(connection, &pass);

	if (user)
	{
		client = client_manager_get_client(atoi(user));
		if (!client)
		{
		//TODO: password authentication
		//TODO: reauthenticate
		printf("Reauthenticating...%s\n", user);
		return respond_authenticate(connection);
		}
	}

	if (!strcmp(method, "GET") &&
		!user &&
		!strcmp(url, "/client"))
	{
		//create new client
		client = client_manager_create_client();

		//serialize client info
		JsonObject* auth = json_object_new();
		json_object_set_int_member(auth, "ID", client->ID);
		json_object_set_string_member(auth, "Password", "PASSWORD");

		JsonNode* node = json_node_new(JSON_NODE_OBJECT);
		json_node_set_object(node, auth);

		JsonGenerator* gen = json_generator_new();
		json_generator_set_root(gen, node);
		gsize size;
		gchar* dat = json_generator_to_data(gen, &size);

		g_object_unref(gen);
		json_node_free(node);
		json_object_unref(auth);

		//send client info
		response = MHD_create_response_from_buffer(size, dat, MHD_RESPMEM_MUST_COPY);
		ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
		MHD_destroy_response(response);

		//cleanup
		g_free(dat);
		return ret;
	}

	if (!user)
		return respond_authenticate(connection);

	if (!strcmp(method, "GET"))
	{
		if (!strcmp(url, "/pollers"))
		{
			gsize size;
			gchar* res = darxend_client_serialize_pollers(client, &size);
			response = MHD_create_response_from_buffer(size, res, MHD_RESPMEM_MUST_COPY);
			ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
			MHD_destroy_response(response);
			g_free(res);
		}
		else
		{
			ret = respond_fail(connection);
		}
	}
	else if (!strcmp(method, "PUT"))
	{
		gchar** params = g_strsplit(url+1, "/", -1);
		guint len = g_strv_length(params); 
		if (len==3 && !strcmp(params[0], "pollers"))
		{
			char* site = params[1];
			char* product = params[2];
			darxend_client_add_poller(client, site, product);
			//TODO: handle errors (and raise them)
			ret = respond_success(connection);
		}
		else
		{
			ret = respond_fail(connection);
		}
		g_strfreev(params);
	}
	else if (!strcmp(method, "DELETE"))
	{
		gchar** params = g_strsplit(url+1, "/", -1);
		guint len = g_strv_length(params); 
		if (len==1 && !strcmp(params[0], "client"))
		{
			int res = client_manager_kill_client(client->ID);

			if (res)
			{
				response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
				ret = MHD_queue_response(connection, MHD_HTTP_FORBIDDEN, response);
				MHD_destroy_response(response);
			}
			else
			{
				ret = respond_success(connection);
			}
		}
		else if (len==3 && !strcmp(params[0], "pollers"))
		{
			char* site = params[1];
			char* product = params[2];
			darxend_client_remove_poller(client, site, product);
			//TODO: handle errors (and raise them)
			ret = respond_success(connection);
		}
		else
		{
			ret = respond_fail(connection);
		}
		g_strfreev(params);
	}
	else if (!strcmp(method, "HEAD"))
	{
	}
	else
	{
		ret = respond_fail(connection);
	}

	return ret;

}

int restapi_init()
{
  d = MHD_start_daemon( MHD_USE_THREAD_PER_CONNECTION,
						PORT,
						NULL,
						NULL,
						&handle_request,
						NULL, //PAGE,
						MHD_OPTION_END);
  return d == NULL;
}

int restapi_shutdown()
{
	MHD_stop_daemon(d);
	return 0;
}


/* libdarxenRestClient.c
 *
 * Copyright (C) 2011 - Kevin Wells <kevin@darxen.org>
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

#include "libdarxenRestfulClient.h"

#include <glib.h>
#include <json-glib/json-glib.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

G_DEFINE_TYPE(DarxenRestfulClient, darxen_restful_client, G_TYPE_OBJECT)

#define USING_PRIVATE(obj) DarxenRestfulClientPrivate* priv = DARXEN_RESTFUL_CLIENT_GET_PRIVATE(obj)
#define DARXEN_RESTFUL_CLIENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_RESTFUL_CLIENT, DarxenRestfulClientPrivate))

typedef struct _DarxenRestfulClientPrivate		DarxenRestfulClientPrivate;
struct _DarxenRestfulClientPrivate
{
	int ID;
	gchar* password;

	gchar* auth_token;
};

typedef struct {
	char* data;
	size_t len;
} ResponseBody;

static void darxen_restful_client_finalize(GObject* gobject);

static CURL*		create_curl_client(const char* method, const char* url, const char* auth_token, ResponseBody* body);
static inline int	validate_response(CURL* curl, GError** error);
static inline int	go_curl(CURL* curl, GError** error);
static size_t		mem_read(void *ptr, size_t size, size_t nmemb, void *stream);

static void
darxen_restful_client_class_init(DarxenRestfulClientClass* klass)
{
	g_type_class_add_private(klass, sizeof(DarxenRestfulClientPrivate));
	
	klass->parent_class.finalize = darxen_restful_client_finalize;
}

static void
darxen_restful_client_init(DarxenRestfulClient* self)
{
	USING_PRIVATE(self);

	priv->ID = 0;
	priv->password = NULL;
	priv->auth_token = NULL;
}

static void
darxen_restful_client_finalize(GObject* gobject)
{
	DarxenRestfulClient* self = DARXEN_RESTFUL_CLIENT(gobject);
	USING_PRIVATE(self);

	if (priv->password)
		g_free(priv->password);
	if (priv->auth_token)
		g_free(priv->auth_token);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(darxen_restful_client_parent_class)->finalize(gobject);
}

DarxenRestfulClient*
darxen_restful_client_new()
{
	GObject *gobject = g_object_new(DARXEN_TYPE_RESTFUL_CLIENT, NULL);
	DarxenRestfulClient* self = DARXEN_RESTFUL_CLIENT(gobject);

	USING_PRIVATE(self);

	return (DarxenRestfulClient*)gobject;
}

int 
darxen_restful_client_connect(DarxenRestfulClient* self, GError** error)
{
	ResponseBody body = {0,};
	USING_PRIVATE(self);

	CURL* curl = create_curl_client("GET", "/client", NULL, &body);
	g_assert(curl);
	if (go_curl(curl, error))
	{
		free(body.data);
		return 0;
	}

	//parse json
	JsonParser* parser = json_parser_new();
	if (!json_parser_load_from_data(parser, body.data, body.len, error))
	{
		free(body.data);
		g_object_unref(G_OBJECT(parser));
		return 0;
	}

	JsonNode* root = json_parser_get_root(parser);
	JsonObject* obj = json_node_get_object(root);
	priv->ID = json_object_get_int_member(obj, "ID");
	priv->password = g_strdup(json_object_get_string_member(obj, "Password"));

	priv->auth_token = g_strdup_printf("%d:%s", priv->ID, priv->password);
	
	free(body.data);
	g_object_unref(G_OBJECT(parser));
	
	return priv->ID;
}

int
darxen_restful_client_disconnect(DarxenRestfulClient* self, GError** error)
{
	USING_PRIVATE(self);

	CURL* curl = create_curl_client("DELETE", "/client", priv->auth_token, NULL);
	g_assert(curl);
	if (go_curl(curl, error))
		return 1;

	priv->ID = 0;
	g_free(priv->password); priv->password = NULL;
	g_free(priv->auth_token); priv->auth_token = NULL;

	return 0;
}

int
darxen_restful_client_add_poller(DarxenRestfulClient* self, char* site, char* product, GError** error)
{
	USING_PRIVATE(self);

	gchar* url = g_strdup_printf("/pollers/%s/%s", site, product);
	CURL* curl = create_curl_client("PUT", url, priv->auth_token, NULL);
	g_free(url);
	g_assert(curl);

	if (go_curl(curl, error))
		return 1;

	return 0;
}

int
darxen_restful_client_remove_poller(DarxenRestfulClient* self, char* site, char* product, GError** error)
{
	USING_PRIVATE(self);

	gchar* url = g_strdup_printf("/pollers/%s/%s", site, product);
	CURL* curl = create_curl_client("DELETE", url, priv->auth_token, NULL);
	g_free(url);
	g_assert(curl);

	if (go_curl(curl, error))
		return 1;

	return 0;
}

RadarPoller*
darxen_restful_client_list_pollers(DarxenRestfulClient* self, int* size, GError** error)
{
	USING_PRIVATE(self);
	ResponseBody body = {0,};

	gchar* url = g_strdup_printf("/pollers");
	CURL* curl = create_curl_client("GET", url, priv->auth_token, &body);
	g_free(url);
	g_assert(curl);

	if (go_curl(curl, error))
		return NULL;

	//parse json
	JsonParser* parser = json_parser_new();
	if (!json_parser_load_from_data(parser, body.data, body.len, error))
	{
		free(body.data);
		g_object_unref(G_OBJECT(parser));
		return NULL;
	}
	
	JsonNode* root = json_parser_get_root(parser);
	JsonArray* array = json_node_get_array(root);
	int len = json_array_get_length(array);
	if (size)
		*size = len;
	RadarPoller* pollers = (RadarPoller*)calloc(len+1, sizeof(RadarPoller));
	int i;
	for (i = 0; i < len; i++)
	{
		JsonObject* poller = json_array_get_object_element(array, i);
		g_assert(json_object_has_member(poller, "Site"));
		pollers[i].site = g_strdup(json_object_get_string_member(poller, "Site"));
		g_assert(json_object_has_member(poller, "Product"));
		pollers[i].product = g_strdup(json_object_get_string_member(poller, "Product"));
	}
	pollers[i].site = NULL;
	pollers[i].product = NULL;
	
	free(body.data);
	g_object_unref(G_OBJECT(parser));
	return pollers;
}

GQuark
darxen_restful_client_error_quark()
{
	return g_quark_from_static_string("darxen-restful-client-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static CURL*
create_curl_client(const char* method, const char* url, const char* auth_token, ResponseBody* body)
{
	CURL* curl = curl_easy_init();
	gchar* full_url = g_strdup_printf("http://%s%s", "localhost", url);
	curl_easy_setopt(curl, CURLOPT_URL, full_url);
	curl_easy_setopt(curl, CURLOPT_PORT, 4889);
	if (auth_token)
	{
		curl_easy_setopt(curl, CURLOPT_USERPWD, auth_token);
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	}
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	g_free(full_url);
	if (body)
	{
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, mem_read);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, body);
	}
	if (!strcmp(method, "GET"))
	{ }
	else if (!strcmp(method, "PUT"))
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
	else if (!strcmp(method, "DELETE"))
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
	else if (!strcmp(method, "HEAD"))
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
	else if (!strcmp(method, "POST"))
		curl_easy_setopt(curl, CURLOPT_POST, 1);
	else
	{
		curl_easy_cleanup(curl);
		return NULL;
	}
	return curl;
}

static inline int
validate_response(CURL* curl, GError** error)
{
	long http_code;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	int code_class = http_code / 100;
	if (code_class != 2)
	{
		g_set_error(error,	DARXEN_RESTFUL_CLIENT_ERROR,
					DARXEN_RESTFUL_CLIENT_ERROR_SERVER_RESPONSE,
					"Server returned error code: %ld", http_code);
		return 1;
	}
	return 0;
}

static inline int
go_curl(CURL* curl, GError** error)
{
	CURLcode code;
	if ((code = curl_easy_perform(curl)) != CURLE_OK)
	{
		curl_easy_cleanup(curl);
		g_set_error(error,	DARXEN_RESTFUL_CLIENT_ERROR,
					DARXEN_RESTFUL_CLIENT_ERROR_CURL,
					"CURL returned error code: %d", code);
		return 1;
	}
	int res = validate_response(curl, error);
	curl_easy_cleanup(curl);
	return res;
}

static size_t
mem_read(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t len = size * nmemb;
	ResponseBody *mem = (ResponseBody*)stream;
	if (!mem)
		return 0;
	mem->data = (char*)realloc(mem->data, mem->len + len + 1);
	if (!mem->data)
		return 0;
	memcpy(&(mem->data[mem->len]), ptr, len);
	mem->len += len;
	mem->data[mem->len] = 0x00;
	return len;
}

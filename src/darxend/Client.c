/* Client.c
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


#include "Client.h"
#include "ClientManager.h"
#include "RadarDataManager.h"
#include "Callbacks.h"

//TODO: threading locks (poller list)

#include <errno.h>
#include <ctype.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define PRUNE_TIME (60 * 5)


G_DEFINE_TYPE(DarxendClient, darxend_client, G_TYPE_OBJECT)
static void darxend_client_finalize(GObject* gobject);

#define USING_PRIVATE(obj) DarxendClientPrivate* priv = DARXEND_CLIENT_GET_PRIVATE(obj);
#define DARXEND_CLIENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEND_TYPE_CLIENT, DarxendClientPrivate))

typedef struct _DarxendClientPrivate		DarxendClientPrivate;
struct _DarxendClientPrivate
{
	GHashTable* table;
	GSList* searches;
	GQueue* pollQueue; //RadarDataInfo

	pthread_mutex_t lockQueue;
	pthread_cond_t condQueue;
};

static void client_poller_iter_next(char* site, char* product, gpointer data);
static gboolean watch_key_value_remove(gpointer key, gpointer value, gpointer data);

static void
darxend_client_class_init(DarxendClientClass* klass)
{
	g_type_class_add_private(klass, sizeof(DarxendClientPrivate));

	klass->parent_class.finalize = darxend_client_finalize;
}

static void
darxend_client_init(DarxendClient* self)
{
	USING_PRIVATE(self)

	/* initialize fields generically here */
	priv->table = NULL;
	priv->pollQueue = NULL;
	priv->searches = NULL;

	self->password = NULL;
}

static void
darxend_client_finalize(GObject* gobject)
{
	DarxendClient* self = DARXEND_CLIENT(gobject);
	USING_PRIVATE(self);

	g_hash_table_foreach_remove(priv->table, watch_key_value_remove, self);
	GSList* list = priv->searches;
	while (list)
	{
		radar_data_manager_free_search(GPOINTER_TO_INT(list->data));
		list = list->next;
	}
	g_slist_free(priv->searches);

	g_free(self->password);

	pthread_mutex_destroy(&priv->lockQueue);
	pthread_cond_destroy(&priv->condQueue);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(darxend_client_parent_class)->finalize(gobject);
}

DarxendClient*
darxend_client_new(int id)
{
	GObject *gobject = (GObject*)g_object_new(DARXEND_TYPE_CLIENT, NULL);
	DarxendClient* self = DARXEND_CLIENT(gobject);

	USING_PRIVATE(self)

	self->ID = id;
	self->intTime = time(0);

	priv->table = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);
	priv->searches = NULL;
	priv->pollQueue = g_queue_new();

	int i;
	self->password = g_strdup("xxxxxxxx");
	for (i = 0; i < 8; i++)
	{
		self->password[i] = g_random_int_range('a', 'z'+1);
		if (g_random_boolean())
			self->password[i] = toupper(self->password[i]);
	}

	pthread_mutex_init(&priv->lockQueue, NULL);
	pthread_cond_init(&priv->condQueue, NULL);

	return (DarxendClient*)gobject;
}


GQuark
darxend_client_error_quark()
{
	return g_quark_from_static_string("darxend-client-error-quark");
}

/*********************
 * Private Functions *
 *********************/

DateTime
timet_to_datetime(time_t rawtime)
{
	struct tm* utc = gmtime(&rawtime);
	DateTime dtm;
	dtm.date.year = utc->tm_year + 1900;
	dtm.date.month = utc->tm_mon;
	dtm.date.day = utc->tm_wday;
	dtm.time.hour = utc->tm_hour;
	dtm.time.minute = utc->tm_min;

	return dtm;
}

void
darxend_client_add_poller(DarxendClient* self, char* site, char* product)
{
	USING_PRIVATE(self);
	site = g_ascii_strdown(site, -1);
	product = g_ascii_strup(product, -1);

	gpointer data = g_hash_table_lookup(priv->table, site);
	GSList* products = (GSList*)data;
	products = g_slist_append(products, strdup(product));
	if (!data)
		g_hash_table_insert(priv->table, strdup(site), products);

	//TODO: FINISH
	//FIXME: time zone dependent?
	time_t now = time(NULL);

	DateTime end = timet_to_datetime(now);
	DateTime start = timet_to_datetime(now - 60*60);

	int searchID = darxend_client_search(self, site, product, &start, &end);

	int recordCount = radar_data_manager_get_search_size(searchID);

	DateTime* records = radar_data_manager_get_search_records(searchID, 0, recordCount);

	int i;
	for (i = MAX(recordCount - 5, 0); i < recordCount; i++)
	{
		DateTime record = records[i];
		darxend_client_add_to_queue(self, site, product, record.date.year, record.date.month, record.date.day, record.time.hour, record.time.minute);
	}

	free(records);
	radar_data_manager_free_search(searchID);

	radar_data_manager_add_poller(self, site, product);

	g_free(site);
	g_free(product);
}

void
darxend_client_remove_poller(DarxendClient* self, char* site, char* product)
{
	USING_PRIVATE(self)
	site = g_ascii_strdown(site, -1);
	product = g_ascii_strup(product, -1);

	gpointer data = g_hash_table_lookup(priv->table, site);
	GSList* products = (GSList*)data;

	if (!data)
	{
		g_warning("Client attempted to remove an invalid poller: %s/%s", site, product);
		g_free(site);
		g_free(product);
		return;
	}

	GSList* rem = g_slist_find_custom(products, product, (GCompareFunc)g_strcmp0);
	products = g_slist_remove_link(products, rem);
	free(rem->data);
	g_slist_free(rem);

	if (!products)
	{
		g_hash_table_remove(priv->table, site);
	}
	else if (products != data)
	{
		g_hash_table_insert(priv->table, strdup(site), products);
	}
	radar_data_manager_remove_poller(self, site, product);
	g_free(site);
	g_free(product);
}

void
darxend_client_validate(DarxendClient* self)
{
	self->intTime = time(0);
}

void
darxend_client_invalidate(DarxendClient* self)
{
	self->intTime = 0;
}

gboolean
darxend_client_is_valid(DarxendClient* self)
{
	return ((time(0) - self->intTime) < PRUNE_TIME);
}

int
darxend_client_search(DarxendClient* self, char* site, char* product, DateTime* start, DateTime* end)
{
	USING_PRIVATE(self)

	int result = radar_data_manager_search(site, product, start, end);
	priv->searches = g_slist_prepend(priv->searches, GINT_TO_POINTER(result));
	return result;
}

gboolean
darxend_client_search_free(DarxendClient* self, int id)
{
	USING_PRIVATE(self);

	//stop clients from deleting others' searches
	if (!g_slist_find(priv->searches, GINT_TO_POINTER(id)))
		return FALSE;

	priv->searches = g_slist_remove(priv->searches, GINT_TO_POINTER(id));
	return radar_data_manager_free_search(id);
}

void
darxend_client_add_to_queue(DarxendClient* self, char* site, char* product, int year, int month, int day, int hour, int minute)
{
	USING_PRIVATE(self)

	RadarDataInfo* info = (RadarDataInfo*)malloc(sizeof(RadarDataInfo));
	info->product = strdup(product);
	info->site = strdup(site);
	info->date.date.year = year;
	info->date.date.month = month;
	info->date.date.day = day;
	info->date.time.hour = hour;
	info->date.time.minute = minute;

	pthread_mutex_lock(&priv->lockQueue);
	g_queue_push_tail(priv->pollQueue, info);
	pthread_cond_broadcast(&priv->condQueue);
	pthread_mutex_unlock(&priv->lockQueue);

	g_debug("Added product to queue: %s/%s %d/%d/%d %d:%d", site, product, month, day, year, hour, minute);
}

int
darxend_client_get_queue_length(DarxendClient* self)
{
	USING_PRIVATE(self)

	pthread_mutex_lock(&priv->lockQueue);
	int length = g_queue_get_length(priv->pollQueue);
	pthread_mutex_unlock(&priv->lockQueue);
	return length;
}

int
darxend_client_wait_queue_length(DarxendClient* self)
{
	USING_PRIVATE(self)

	int length;
	pthread_mutex_lock(&priv->lockQueue);
	{
		struct timeval now;
		struct timespec timeout;
		int retcode;

		gettimeofday(&now, NULL);
		timeout.tv_sec = now.tv_sec + 30; //30 seconds
		timeout.tv_nsec = now.tv_usec * 1000;
		retcode = 0;
		while (darxend_client_is_valid(self) && ((length = g_queue_get_length(priv->pollQueue)) == 0) && retcode != ETIMEDOUT)
		{
			retcode = pthread_cond_timedwait(&priv->condQueue, &priv->lockQueue, &timeout);
		}
	}
	pthread_mutex_unlock(&priv->lockQueue);
	return length;
}

RadarDataInfo*
darxend_client_read_queue(DarxendClient* self, int count)
{
	if (count > darxend_client_get_queue_length(self))
		return NULL;

	USING_PRIVATE(self)

	pthread_mutex_lock(&priv->lockQueue);

	RadarDataInfo* result = (RadarDataInfo*)calloc(count, sizeof(RadarDataInfo));
	int i;
	for (i = 0; i < count; i++)
	{
		RadarDataInfo* info = (RadarDataInfo*)g_queue_pop_head(priv->pollQueue);
		result[i] = *info;
		free(info);
	}

	pthread_mutex_unlock(&priv->lockQueue);

	return result;
}

gchar*
darxend_client_serialize_pollers(DarxendClient* self, gsize* size)
{
	JsonArray* array = json_array_new();
	radar_data_manager_iter_pollers(self, client_poller_iter_next, array);

	JsonNode* node = json_node_new(JSON_NODE_ARRAY);
	json_node_set_array(node, array);

	JsonGenerator* gen = json_generator_new();
	json_generator_set_root(gen, node);
	gchar* dat = json_generator_to_data(gen, size);

	g_object_unref(gen);
	json_node_free(node);
	json_array_unref(array);

	return dat;
}

void
client_poller_iter_next(char* site, char* product, gpointer data)
{
	JsonArray* array = (JsonArray*)data;

	JsonObject* poller = json_object_new();
	json_object_set_string_member(poller, "Site", site);
	json_object_set_string_member(poller, "Product", product);

	JsonNode* node = json_node_new(JSON_NODE_OBJECT);
	json_node_set_object(node, poller);

	json_array_add_element(array, node); //does not ref node

	json_object_unref(poller);
}

gboolean
watch_key_value_remove(gpointer key, gpointer value, gpointer data)
{
	DarxendClient* client = (DarxendClient*)data;
	char* site = (char*)key;
	GSList* products = (GSList*)value;
	GSList* pproducts = products;
	char* product;

	while (pproducts)
	{
		product = (char*)pproducts->data;

		radar_data_manager_remove_poller(client, site, product);
		free(product);

		pproducts = pproducts->next;
	}
	g_slist_free(products);

	return TRUE;
}

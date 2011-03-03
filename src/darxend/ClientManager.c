/* ClientManager.cc
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


#include "ClientManager.h"

#include <glib.h>

#include <pthread.h>
#include <malloc.h>

static int intClientID;
static GSList* clients;

static pthread_mutex_t lockClients;

static gboolean pruneClients(gpointer data);
static void pruneClient(gpointer client, gpointer data);

void
client_manager_init()
{
	clients = NULL;
	intClientID = 1;
	pthread_mutex_init(&lockClients, NULL);

	g_timeout_add_seconds(30, pruneClients, NULL);
}

DarxendClient*
client_manager_create_client()
{
	pthread_mutex_lock(&lockClients);

	DarxendClient* client = darxend_client_new(intClientID);
	clients = g_slist_append(clients, client);
	intClientID++;

	pthread_mutex_unlock(&lockClients);

	return client;
}

gboolean
client_manager_validate_client(int ID)
{
	DarxendClient* client = client_manager_get_client(ID);
	if (!client)
		return FALSE;
	darxend_client_validate(client);
	return TRUE;
}

int
client_manager_kill_client(int ID)
{
	DarxendClient* client = client_manager_get_client(ID);
	if (!client)
	{
		g_warning("Unable to find client to kill: %i", ID);
		return 1;
	}
	darxend_client_invalidate(client);
	pruneClient(client, NULL);
	return 0;
}

DarxendClient*
client_manager_get_client(int ID)
{
	pthread_mutex_lock(&lockClients);

	GSList* pclients = clients;
	while (pclients)
	{
		DarxendClient* client = (DarxendClient*)pclients->data;
		if (client->ID == ID)
			break;
		pclients = pclients->next;
	}

	if (!pclients)
	{
		pthread_mutex_unlock(&lockClients);
		return NULL;
	}
	DarxendClient* res = (DarxendClient*)pclients->data;

	darxend_client_validate(res);

	pthread_mutex_unlock(&lockClients);

	return res;
}

/*********************
 * Private Functions *
 *********************/

static gboolean
pruneClients(gpointer data)
{
	pthread_mutex_lock(&lockClients);

	g_slist_foreach(clients, pruneClient, NULL);

	pthread_mutex_unlock(&lockClients);

	return TRUE;
}

static void
pruneClient(gpointer pclient, gpointer data)
{
	DarxendClient* client = (DarxendClient*)pclient;
	if (darxend_client_is_valid(client))
		return;
	g_message("Pruning client: %i", client->ID);
	clients = g_slist_remove(clients, client);

	g_object_unref(G_OBJECT(client));
}


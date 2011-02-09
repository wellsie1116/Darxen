/* libdarxenRestClient.h
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

#include <glib-object.h>

G_BEGIN_DECLS

#define DARXEN_RESTFUL_CLIENT_ERROR darxen_restful_client_error_quark()

#define DARXEN_TYPE_RESTFUL_CLIENT				(darxen_restful_client_get_type())
#define DARXEN_RESTFUL_CLIENT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_RESTFUL_CLIENT, DarxenRestfulClient))
#define DARXEN_RESTFUL_CLIENT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_RESTFUL_CLIENT, DarxenRestfulClientClass))
#define DARXEN_IS_RESTFUL_CLIENT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_RESTFUL_CLIENT))
#define DARXEN_IS_RESTFUL_CLIENT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_RESTFUL_CLIENT))
#define DARXEN_RESTFUL_CLIENT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_RESTFUL_CLIENT, DarxenRestfulClientClass))

typedef struct _DarxenRestfulClient			DarxenRestfulClient;
typedef struct _DarxenRestfulClientClass		DarxenRestfulClientClass;

struct _DarxenRestfulClient
{
	GObject parent;
};

struct _DarxenRestfulClientClass
{
	GObjectClass parent_class;
};

typedef enum
{
	DARXEN_RESTFUL_CLIENT_ERROR_CURL,
	DARXEN_RESTFUL_CLIENT_ERROR_SERVER_RESPONSE,
	DARXEN_RESTFUL_CLIENT_ERROR_FAILED
} DarxenRestfulClientError;

typedef struct {
	gchar* site;
	gchar* product;
} RadarPoller;

GType					darxen_restful_client_get_type	() G_GNUC_CONST;
DarxenRestfulClient*	darxen_restful_client_new		();

/* Public functions here */
int						darxen_restful_client_connect		(DarxenRestfulClient* self, GError** error);
int						darxen_restful_client_disconnect	(DarxenRestfulClient* self, GError** error);

int						darxen_restful_client_add_poller	(DarxenRestfulClient* self, char* site, char* product, GError** error);
int						darxen_restful_client_remove_poller	(DarxenRestfulClient* self, char* site, char* product, GError** error);
RadarPoller*			darxen_restful_client_list_pollers	(DarxenRestfulClient* self, int* size, GError** error);


GQuark			darxen_restful_client_error_quark	();

G_END_DECLS



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

#define DARXEN_POLLER_ERROR darxen_poller_error_quark()

#define DARXEN_TYPE_POLLER				(darxen_poller_get_type())
#define DARXEN_POLLER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_POLLER, DarxenPoller))
#define DARXEN_POLLER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_POLLER, DarxenPollerClass))
#define DARXEN_IS_POLLER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_POLLER))
#define DARXEN_IS_POLLER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_POLLER))
#define DARXEN_POLLER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_POLLER, DarxenPollerClass))

typedef struct _DarxenRestfulClient			DarxenRestfulClient;
typedef struct _DarxenRestfulClientClass		DarxenRestfulClientClass;

typedef struct _DarxenPoller			DarxenPoller;
typedef struct _DarxenPollerClass		DarxenPollerClass;

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

typedef struct {
	gchar* site;
	gchar* product;
	gchar* ID;
	gchar* data;
} RadarData;

GType					darxen_restful_client_get_type	() G_GNUC_CONST;
DarxenRestfulClient*	darxen_restful_client_new		();

/* Public functions here */
int						darxen_restful_client_connect		(DarxenRestfulClient* self, GError** error);
int						darxen_restful_client_disconnect	(DarxenRestfulClient* self, GError** error);


DarxenPoller*			darxen_restful_client_add_poller	(	DarxenRestfulClient* self, 
																gchar* site, 
																gchar* product, 
																GError** error);

int						darxen_restful_client_remove_poller	(	DarxenRestfulClient* self, 
																gchar* site, 
																gchar* product, 
																GError** error);

RadarPoller*			darxen_restful_client_list_pollers	(	DarxenRestfulClient* self, 
																int* size, 
																GError** error);


gboolean				darxen_restful_client_search_data	(	DarxenRestfulClient* self, 
																const gchar* site, 
																const gchar* product, 
																const gchar* startId, 
																const gchar* endId, 
																int* searchId, 
																int* count,
																GError** error);

gchar**					darxen_restful_client_read_search	(	DarxenRestfulClient* self,
																int searchId, 
																int start, 
																int count, 
																GError** error);

gboolean				darxen_restful_client_free_search	(	DarxenRestfulClient* self, 
																int searchId, 
																GError** error);

char*					darxen_restful_client_read_data		(	DarxenRestfulClient* self,
																const gchar* site,
																const gchar* product,
																const gchar* id,
																GError** error);



//RadarDataInfo*			darxen_restful_client_get_data		(DarxenRestfulClient* self, int len);


GQuark			darxen_restful_client_error_quark	();

struct _DarxenPoller
{
	GObject parent;
};

struct _DarxenPollerClass
{
	GObjectClass parent_class;
	
	/* signals */
	void (*data_received) (DarxenPoller* poller, RadarData* data);

	/* virtual funcs */
};

typedef enum
{
	DARXEN_POLLER_ERROR_FAILED
} DarxenPollerError;

GType			darxen_poller_get_type	() G_GNUC_CONST;
DarxenPoller*	darxen_poller_new		(DarxenRestfulClient* client, const gchar* site, const gchar* product);

/* Public functions here */

GQuark			darxen_poller_error_quark	();

G_END_DECLS



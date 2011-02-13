/* Client.h
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

#ifndef CLIENT_H_
#define CLIENT_H_

#include "restapi.h"

#include <glib.h>
#include <pthread.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define DARXEND_CLIENT_ERROR darxend_client_error_quark()

#define DARXEND_TYPE_CLIENT				(darxend_client_get_type())
#define DARXEND_CLIENT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEND_TYPE_CLIENT, DarxendClient))
#define DARXEND_CLIENT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEND_TYPE_CLIENT, DarxendClientClass))
#define DARXEND_IS_CLIENT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEND_TYPE_CLIENT))
#define DARXEND_IS_CLIENT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEND_TYPE_CLIENT))
#define DARXEND_CLIENT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEND_TYPE_CLIENT, DarxendClientClass))

typedef struct _DarxendClient			DarxendClient;
typedef struct _DarxendClientClass		DarxendClientClass;

struct _DarxendClient
{
	GObject parent;

	int ID;
	long int intTime;

	gchar* password;
};

struct _DarxendClientClass
{
	GObjectClass parent_class;

	/* signals */
	/* virtual funcs */
};

typedef enum
{
	DARXEND_CLIENT_ERROR_FAILED
} DarxendClientError;

GType			darxend_client_get_type	() G_GNUC_CONST;
DarxendClient*	darxend_client_new		(int id);

void			darxend_client_add_poller			(DarxendClient* self, char* site, char* product);
void			darxend_client_remove_poller		(DarxendClient* self, char* site, char* product);

void			darxend_client_validate				(DarxendClient* self);
void			darxend_client_invalidate			(DarxendClient* self);
gboolean		darxend_client_is_valid				(DarxendClient* self);

int				darxend_client_search				(DarxendClient* self, char* site, char* product, DateTime* start, DateTime* end);
gboolean		darxend_client_search_free			(DarxendClient* self, int id);

void			darxend_client_add_to_queue			(DarxendClient* self, char* site, char* product, int year, int month, int day, int hour, int minute);
int				darxend_client_get_queue_length		(DarxendClient* self);
int				darxend_client_wait_queue_length	(DarxendClient* self);
RadarDataInfo*	darxend_client_read_queue			(DarxendClient* self, int count);

gchar*			darxend_client_serialize_pollers	(DarxendClient* self, gsize* size);

GQuark			darxend_client_error_quark			();

G_END_DECLS

#endif /*CLIENT_H_*/

/* libdarxenPoller.c
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

G_DEFINE_TYPE(DarxenPoller, darxen_poller, G_TYPE_OBJECT)

#define USING_PRIVATE(obj) DarxenPollerPrivate* priv = DARXEN_POLLER_GET_PRIVATE(obj)
#define DARXEN_POLLER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_POLLER, DarxenPollerPrivate))

enum
{
	DATA_RECEIVED,
	LAST_SIGNAL
};

typedef struct _DarxenPollerPrivate		DarxenPollerPrivate;
struct _DarxenPollerPrivate
{
	DarxenRestfulClient* client;
};

static guint signals[LAST_SIGNAL] = {0,};

static void darxen_poller_finalize(GObject* gobject);

static void
darxen_poller_class_init(DarxenPollerClass* klass)
{
	g_type_class_add_private(klass, sizeof(DarxenPollerPrivate));
	
	signals[DATA_RECEIVED] = g_signal_new(
			"data-received",
			G_TYPE_FROM_CLASS(G_OBJECT_CLASS(klass)),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(DarxenPollerClass, data_received),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1, G_TYPE_POINTER);
	
	klass->parent_class.finalize = darxen_poller_finalize;
}

static void
darxen_poller_init(DarxenPoller* self)
{
	USING_PRIVATE(self);

	priv->client = NULL;
	self->site = NULL;
	self->product = NULL;
}

static void
darxen_poller_finalize(GObject* gobject)
{
	DarxenPoller* self = DARXEN_POLLER(gobject);

	g_free(self->site);
	g_free(self->product);
	
	G_OBJECT_CLASS(darxen_poller_parent_class)->finalize(gobject);
}

DarxenPoller*
darxen_poller_new(DarxenRestfulClient* client, const gchar* site, const gchar* product)
{
	GObject *gobject = g_object_new(DARXEN_TYPE_POLLER, NULL);
	DarxenPoller* self = DARXEN_POLLER(gobject);

	USING_PRIVATE(self);

	priv->client = client;
	self->site = g_strdup(site);
	self->product = g_strdup(product);

	return (DarxenPoller*)gobject;
}

void
darxen_poller_notify_data(DarxenPoller* poller, RadarData* data)
{
	g_return_if_fail(DARXEN_IS_POLLER(poller));

	g_signal_emit(poller, signals[DATA_RECEIVED], 0, data);
}


GQuark
darxen_poller_error_quark()
{
	return g_quark_from_static_string("darxen-poller-error-quark");
}

/*********************
 * Private Functions *
 *********************/


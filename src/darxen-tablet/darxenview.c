/* darxenview.c
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

#include "darxenview.h"

#include "darxenconfig.h"

G_DEFINE_TYPE(DarxenView, darxen_view, GLTK_TYPE_VBOX)

#define USING_PRIVATE(obj) DarxenViewPrivate* priv = DARXEN_VIEW_GET_PRIVATE(obj)
#define DARXEN_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_VIEW, DarxenViewPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _DarxenViewPrivate		DarxenViewPrivate;
struct _DarxenViewPrivate
{
	DarxenViewInfo* viewInfo;

};

//static guint signals[LAST_SIGNAL] = {0,};

static void darxen_view_dispose(GObject* gobject);
static void darxen_view_finalize(GObject* gobject);

static void
darxen_view_class_init(DarxenViewClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(DarxenViewPrivate));
	
	gobject_class->dispose = darxen_view_dispose;
	gobject_class->finalize = darxen_view_finalize;
}

static void
darxen_view_init(DarxenView* self)
{
	USING_PRIVATE(self);

	priv->viewInfo = NULL;
}

static void
darxen_view_dispose(GObject* gobject)
{
	DarxenView* self = DARXEN_VIEW(gobject);
	USING_PRIVATE(self);

	//free and release references

	G_OBJECT_CLASS(darxen_view_parent_class)->dispose(gobject);
}

static void
darxen_view_finalize(GObject* gobject)
{
	DarxenView* self = DARXEN_VIEW(gobject);
	USING_PRIVATE(self);

	//free memory

	G_OBJECT_CLASS(darxen_view_parent_class)->finalize(gobject);
}

GltkWidget*
darxen_view_new(DarxenViewInfo* viewInfo)
{
	GObject *gobject = g_object_new(DARXEN_TYPE_VIEW, NULL);
	DarxenView* self = DARXEN_VIEW(gobject);

	USING_PRIVATE(self);

	priv->viewInfo = viewInfo;

	gchar* desc = g_strdup_printf("View entited: %s", viewInfo->name);
	gltk_box_append_widget(GLTK_BOX(self), gltk_label_new(desc), TRUE, TRUE);
	g_free(desc);

	return (GltkWidget*)gobject;
}


GQuark
darxen_view_error_quark()
{
	return g_quark_from_static_string("darxen-view-error-quark");
}

/*********************
 * Private Functions *
 *********************/



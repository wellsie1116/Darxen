/* darxenviewconfig.c
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

#include "darxenviewconfig.h"


G_DEFINE_TYPE(DarxenViewConfig, darxen_view_config, GLTK_TYPE_VBOX)

#define USING_PRIVATE(obj) DarxenViewConfigPrivate* priv = DARXEN_VIEW_CONFIG_GET_PRIVATE(obj)
#define DARXEN_VIEW_CONFIG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_VIEW_CONFIG, DarxenViewConfigPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _DarxenViewConfigPrivate		DarxenViewConfigPrivate;
struct _DarxenViewConfigPrivate
{
	gchar* site;
	DarxenViewInfo* viewInfo;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void darxen_view_config_dispose(GObject* gobject);
static void darxen_view_config_finalize(GObject* gobject);

static void
darxen_view_config_class_init(DarxenViewConfigClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(DarxenViewConfigPrivate));
	
	gobject_class->dispose = darxen_view_config_dispose;
	gobject_class->finalize = darxen_view_config_finalize;
}

static void
darxen_view_config_init(DarxenViewConfig* self)
{
	USING_PRIVATE(self);

	priv->site = NULL;
	priv->viewInfo = NULL;
}

static void
darxen_view_config_dispose(GObject* gobject)
{
	G_OBJECT_CLASS(darxen_view_config_parent_class)->dispose(gobject);
}

static void
darxen_view_config_finalize(GObject* gobject)
{
	DarxenViewConfig* self = DARXEN_VIEW_CONFIG(gobject);
	USING_PRIVATE(self);

	free(priv->site);

	G_OBJECT_CLASS(darxen_view_config_parent_class)->finalize(gobject);
}

GltkWidget*
darxen_view_config_new(gchar* site, DarxenViewInfo* viewInfo)
{
	GObject *gobject = g_object_new(DARXEN_TYPE_VIEW_CONFIG, NULL);
	DarxenViewConfig* self = DARXEN_VIEW_CONFIG(gobject);

	USING_PRIVATE(self);

	priv->site = g_strdup(site);
	priv->viewInfo = viewInfo;

	GltkWidget* description = gltk_label_new("This is a view configuration window!");
	//Change Name - Text box
	//Change Product - Selectable/Scrollable List box OR Combo box
	//Change Source Type - Radio buttons
	//Select Date Range - Text Box (with numeric filter) OR Number spinners

	gltk_box_append_widget(GLTK_BOX(self), description, TRUE, TRUE);

	return (GltkWidget*)gobject;
}


GQuark
darxen_view_config_error_quark()
{
	return g_quark_from_static_string("darxen-view-config-error-quark");
}

/*********************
 * Private Functions *
 *********************/


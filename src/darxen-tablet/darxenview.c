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
#include "darxenradarviewer.h"

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
	gchar* site;
	DarxenViewInfo* viewInfo;
	GltkWidget* label;

	DarxenRadarViewer* radarViewer;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void darxen_view_dispose(GObject* gobject);
static void darxen_view_finalize(GObject* gobject);

static void		config_viewNameChanged		(	DarxenConfig* config,
												const gchar* site,
												DarxenViewInfo* viewInfo,
												const gchar* oldName,
												DarxenView* view);

static void		config_viewUpdated			(	DarxenConfig* config,
												const gchar* site,
												DarxenViewInfo* viewInfo,
												DarxenView* view);

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

	priv->site = NULL;
	priv->viewInfo = NULL;
	priv->label = NULL;
	priv->radarViewer = NULL;
}

static void
darxen_view_dispose(GObject* gobject)
{
	DarxenView* self = DARXEN_VIEW(gobject);
	USING_PRIVATE(self);

	if (priv->label)
	{
		g_object_unref(priv->label);
		priv->label = NULL;
	}
	
	if (priv->radarViewer)
	{
		g_object_unref(priv->radarViewer);
		priv->radarViewer = NULL;
	}

	G_OBJECT_CLASS(darxen_view_parent_class)->dispose(gobject);
}

static void
darxen_view_finalize(GObject* gobject)
{
	DarxenView* self = DARXEN_VIEW(gobject);
	USING_PRIVATE(self);

	g_free(priv->site);

	G_OBJECT_CLASS(darxen_view_parent_class)->finalize(gobject);
}

GltkWidget*
darxen_view_new(const gchar* site, DarxenViewInfo* viewInfo)
{
	GObject *gobject = g_object_new(DARXEN_TYPE_VIEW, NULL);
	DarxenView* self = DARXEN_VIEW(gobject);
	DarxenConfig* config = darxen_config_get_instance();

	USING_PRIVATE(self);

	priv->site = g_strdup(site);
	priv->viewInfo = viewInfo;

	gchar* desc = g_strdup_printf("View entited: %s", viewInfo->name);
	priv->label = gltk_label_new(desc);
	g_object_ref(priv->label);
	gltk_box_append_widget(GLTK_BOX(self), priv->label, FALSE, FALSE);
	g_free(desc);

	priv->radarViewer = darxen_radar_viewer_new(site, viewInfo);
	g_object_ref(G_OBJECT(priv->radarViewer));
	gltk_box_append_widget(GLTK_BOX(self), GLTK_WIDGET(priv->radarViewer), TRUE, TRUE);
	
	g_signal_connect(config, "view-name-changed", (GCallback)config_viewNameChanged, self);
	g_signal_connect(config, "view-updated", (GCallback)config_viewUpdated, self);

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

static void
config_viewNameChanged(	DarxenConfig* config,
						const gchar* site,
						DarxenViewInfo* viewInfo,
						const gchar* oldName,
						DarxenView* view)
{
	USING_PRIVATE(view);

	if (priv->viewInfo != viewInfo)
		return;

	gchar* desc = g_strdup_printf("View entited: %s", viewInfo->name);
	gltk_label_set_text(GLTK_LABEL(priv->label), desc);
	g_free(desc);
}

static void		
config_viewUpdated(	DarxenConfig* config,
					const gchar* site,
					DarxenViewInfo* viewInfo,
					DarxenView* view)
{
}





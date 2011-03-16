/* darxensitelist.c
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

#include "darxensitelist.h"

#include <glib.h>

G_DEFINE_TYPE(DarxenSiteList, darxen_site_list, GLTK_TYPE_LIST)

#define USING_PRIVATE(obj) DarxenSiteListPrivate* priv = DARXEN_SITE_LIST_GET_PRIVATE(obj)
#define DARXEN_SITE_LIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_SITE_LIST, DarxenSiteListPrivate))

enum
{
	//sites reordered
	//views reordered
	//view deleted
	VIEW_SELECTED,
	VIEW_CONFIG,
	LAST_SIGNAL
};

typedef struct _DarxenSiteListPrivate		DarxenSiteListPrivate;
typedef struct _Site						Site;
typedef struct _View						View;

struct _DarxenSiteListPrivate
{
	GHashTable* sites;
};

struct _Site
{
	DarxenSiteList* list;
	gchar* name;
	GltkWidget* siteBox;
	GltkWidget* views;
};

struct _View
{
	Site* site;
	gchar* name;
	GltkWidget* button;
};

static guint signals[LAST_SIGNAL] = {0,};

static void darxen_site_list_dispose(GObject* gobject);
static void darxen_site_list_finalize(GObject* gobject);

static void
darxen_site_list_class_init(DarxenSiteListClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(DarxenSiteListPrivate));

	signals[VIEW_SELECTED] = 
		g_signal_new(	"view-selected",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET(DarxenSiteListClass, view_selected),
						NULL, NULL,
						g_cclosure_user_marshal_VOID__POINTER_POINTER,
						G_TYPE_NONE, 2,
						G_TYPE_POINTER, G_TYPE_POINTER);
	
	signals[VIEW_CONFIG] = 
		g_signal_new(	"view-config",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET(DarxenSiteListClass, view_config),
						NULL, NULL,
						g_cclosure_user_marshal_VOID__POINTER_POINTER,
						G_TYPE_NONE, 2,
						G_TYPE_POINTER, G_TYPE_POINTER);
	
	gobject_class->dispose = darxen_site_list_dispose;
	gobject_class->finalize = darxen_site_list_finalize;
}

static void
delete_site_list_item(GltkListItem* listItem)
{
	gltk_list_remove_item(listItem->list, listItem);

	Site* site = (Site*)listItem->data;
	g_free(site->name);
	g_object_unref(G_OBJECT(site->views));
	g_object_unref(G_OBJECT(site->siteBox));
}

static void
darxen_site_list_init(DarxenSiteList* self)
{
	USING_PRIVATE(self);

	priv->sites = NULL;
}

static void
darxen_site_list_dispose(GObject* gobject)
{
	DarxenSiteList* self = DARXEN_SITE_LIST(gobject);
	USING_PRIVATE(self);

	g_hash_table_destroy(priv->sites);

	G_OBJECT_CLASS(darxen_site_list_parent_class)->dispose(gobject);
}

static void
darxen_site_list_finalize(GObject* gobject)
{
	//DarxenSiteList* self = DARXEN_SITE_LIST(gobject);
	//USING_PRIVATE(self);

	//free memory

	G_OBJECT_CLASS(darxen_site_list_parent_class)->finalize(gobject);
}

GltkWidget*
darxen_site_list_new()
{
	GObject *gobject = g_object_new(DARXEN_TYPE_SITE_LIST, NULL);
	DarxenSiteList* self = DARXEN_SITE_LIST(gobject);

	USING_PRIVATE(self);

	priv->sites = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)delete_site_list_item);

	return (GltkWidget*)gobject;
}

static gboolean
site_clicked(GltkButton* button, GltkEventClick* event, Site* siteInfo)
{
	gltk_widget_set_visible(siteInfo->views, !gltk_widget_get_visible(siteInfo->views));

	return TRUE;
}

void
darxen_site_list_add_site(DarxenSiteList* list, const gchar* site)
{
	g_return_if_fail(DARXEN_IS_SITE_LIST(list));
	USING_PRIVATE(list);
	
	Site* siteInfo = g_new(Site, 1);
	siteInfo->list = list;
	siteInfo->name = g_strdup(site);
	siteInfo->siteBox = gltk_vbox_new();
	siteInfo->views = gltk_list_new();
	g_object_ref(G_OBJECT(siteInfo->siteBox));
	g_object_ref(G_OBJECT(siteInfo->views));

	GltkWidget* siteButton = gltk_button_new(site);
	g_signal_connect(siteButton, "click-event", (GCallback)site_clicked, siteInfo);

	GltkWidget* hbox = gltk_hbox_new();

	gltk_box_append_widget(GLTK_BOX(hbox), gltk_label_new("  "), FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), siteInfo->views, TRUE, TRUE);

	gltk_box_append_widget(GLTK_BOX(siteInfo->siteBox), siteButton, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(siteInfo->siteBox), hbox, TRUE, TRUE);

	GltkListItem* listItem = gltk_list_add_item(GLTK_LIST(list), siteInfo->siteBox, siteInfo);

	g_hash_table_insert(priv->sites, g_strdup(site), listItem);
}

static gboolean
view_clicked(GltkButton* button, GltkEventClick* event, View* viewInfo)
{
	g_signal_emit(G_OBJECT(viewInfo->site->list), signals[VIEW_SELECTED], 0, viewInfo->site->name, viewInfo->name);
	return TRUE;
}

static gboolean
view_slide(GltkButton* button, GltkEventSlide* event, View* viewInfo)
{
	g_signal_emit(G_OBJECT(viewInfo->site->list), signals[VIEW_CONFIG], 0, viewInfo->site->name, viewInfo->name);
	return TRUE;
}

void
darxen_site_list_add_view(DarxenSiteList* list, const gchar* site, const gchar* view)
{
	g_return_if_fail(DARXEN_IS_SITE_LIST(list));
	USING_PRIVATE(list);

	GltkListItem* listItem = (GltkListItem*)g_hash_table_lookup(priv->sites, site);
	g_return_if_fail(listItem);

	Site* siteInfo = (Site*)listItem->data;

	View* viewInfo = g_new(View, 1);
	viewInfo->site = siteInfo;
	viewInfo->name = g_strdup(view);
	viewInfo->button = gltk_slide_button_new(view);
	g_object_ref(G_OBJECT(viewInfo->button));
	g_signal_connect(viewInfo->button, "click-event", (GCallback)view_clicked, viewInfo);
	g_signal_connect(viewInfo->button, "slide-event", (GCallback)view_slide, viewInfo);
	
	gltk_list_add_item(GLTK_LIST(siteInfo->views), viewInfo->button, viewInfo);
}


GQuark
darxen_site_list_error_quark()
{
	return g_quark_from_static_string("darxen-site-list-error-quark");
}

/*********************
 * Private Functions *
 *********************/


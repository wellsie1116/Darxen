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
#include "darxenconfig.h"
#include "darxenmainview.h"

#include <libdarxenRadarSites.h>

#include <glib.h>

G_DEFINE_TYPE(DarxenSiteList, darxen_site_list, GLTK_TYPE_LIST)

#define USING_PRIVATE(obj) DarxenSiteListPrivate* priv = DARXEN_SITE_LIST_GET_PRIVATE(obj)
#define DARXEN_SITE_LIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_SITE_LIST, DarxenSiteListPrivate))

enum
{
	//sites reordered
	//views reordered
	VIEW_SELECTED,
	VIEW_CONFIG,
	SAVE_VIEW_CONFIG,
	REVERT_VIEW_CONFIG,
	VIEW_DESTROYED,

	LAST_SIGNAL
};

typedef struct _DarxenSiteListPrivate		DarxenSiteListPrivate;
typedef struct _Site						Site;
typedef struct _View						View;

struct _DarxenSiteListPrivate
{
	GHashTable* siteMap; //gchar* -> GltkListItem (data = Site)

	GltkWidget* currentConfigButton;
};

struct _Site
{
	DarxenSiteList* list;
	gchar* name;
	GltkWidget* siteBox;
	GltkWidget* views;
	GHashTable* viewMap; //gchar* -> GltkListItem (data = View)
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

static void darxen_site_list_site_inserted(GltkList* list, GltkListItem* item, int index);
static void darxen_site_list_site_moved(GltkList* list, GltkListItem* item, int oldIndex);
static void darxen_site_list_site_deleted(GltkList* list, GltkListItem* item);
static GltkWidget*	darxen_site_list_site_convert_dropped_item(GltkList* list, const gchar* targetType, GltkListItem* item);
static void darxen_site_list_view_deleted(GltkList* viewList, GltkListItem* item, DarxenSiteList* siteList);

static void		config_siteDeleted			(	DarxenConfig* config,
												const gchar* site,
												DarxenSiteList* list);

static void		config_viewDeleted			(	DarxenConfig* config,
												const gchar* site,
												const gchar* view,
												DarxenSiteList* list);

static void		config_viewNameChanged		(	DarxenConfig* config,
												const gchar* site,
												DarxenViewInfo* viewInfo,
												const gchar* newName,
												DarxenSiteList* list);

static void
darxen_site_list_class_init(DarxenSiteListClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkListClass* gltklist_class = GLTK_LIST_CLASS(klass);

	g_type_class_add_private(klass, sizeof(DarxenSiteListPrivate));

	signals[VIEW_SELECTED] = 
		g_signal_new(	"view-selected",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET(DarxenSiteListClass, view_selected),
						NULL, NULL,
						g_cclosure_user_marshal_VOID__STRING_STRING,
						G_TYPE_NONE, 2,
						G_TYPE_STRING, G_TYPE_STRING);
	
	signals[VIEW_CONFIG] = 
		g_signal_new(	"view-config",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET(DarxenSiteListClass, view_config),
						NULL, NULL,
						g_cclosure_user_marshal_VOID__STRING_STRING,
						G_TYPE_NONE, 2,
						G_TYPE_STRING, G_TYPE_STRING);
	
	signals[SAVE_VIEW_CONFIG] = 
		g_signal_new(	"save-view-config",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET(DarxenSiteListClass, save_view_config),
						NULL, NULL,
						g_cclosure_user_marshal_VOID__STRING_STRING,
						G_TYPE_NONE, 2,
						G_TYPE_STRING, G_TYPE_STRING);
	
	signals[REVERT_VIEW_CONFIG] = 
		g_signal_new(	"revert-view-config",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET(DarxenSiteListClass, revert_view_config),
						NULL, NULL,
						g_cclosure_user_marshal_VOID__STRING_STRING,
						G_TYPE_NONE, 2,
						G_TYPE_STRING, G_TYPE_STRING);
	
	signals[VIEW_DESTROYED] = 
		g_signal_new(	"view-destroyed",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET(DarxenSiteListClass, view_destroyed),
						NULL, NULL,
						g_cclosure_user_marshal_VOID__STRING_STRING,
						G_TYPE_NONE, 2,
						G_TYPE_STRING, G_TYPE_STRING);

	gobject_class->dispose = darxen_site_list_dispose;
	gobject_class->finalize = darxen_site_list_finalize;

	gltklist_class->item_inserted = darxen_site_list_site_inserted;
	gltklist_class->item_moved = darxen_site_list_site_moved;
	gltklist_class->item_deleted = darxen_site_list_site_deleted;
	gltklist_class->convert_dropped_item = darxen_site_list_site_convert_dropped_item;
}

static void
delete_site_list_item(GltkListItem* listItem)
{
	//gltk_list_remove_item(listItem->list, listItem);

	Site* site = (Site*)listItem->data;
	g_hash_table_destroy(site->viewMap);
	g_free(site->name);
	g_object_unref(G_OBJECT(site->views));
	g_object_unref(G_OBJECT(site->siteBox));
	g_free(site);
}

static void
delete_view_list_item(GltkListItem* listItem)
{
	gltk_list_remove_item(listItem->list, listItem);
	
	View* view = (View*)listItem->data;
	g_signal_emit(	G_OBJECT(view->site->list), signals[VIEW_DESTROYED], 0,
					view->site->name, view->name);
	
	g_free(view->name);
	g_object_unref(view->button);
}

static void
darxen_site_list_init(DarxenSiteList* self)
{
	USING_PRIVATE(self);

	g_object_set(self, "target-type", "SiteList", NULL);

	priv->siteMap = NULL;

	priv->currentConfigButton = NULL;
}

static void
darxen_site_list_dispose(GObject* gobject)
{
	DarxenSiteList* self = DARXEN_SITE_LIST(gobject);
	USING_PRIVATE(self);

	g_hash_table_destroy(priv->siteMap);

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
	GObject *gobject = g_object_new(DARXEN_TYPE_SITE_LIST, "deletable", TRUE, NULL);
	DarxenSiteList* self = DARXEN_SITE_LIST(gobject);
	DarxenConfig* config = darxen_config_get_instance();

	USING_PRIVATE(self);

	priv->siteMap = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)delete_site_list_item);

	g_signal_connect(config, "view-name-changed", (GCallback)config_viewNameChanged, self);
	g_signal_connect(config, "view-deleted", (GCallback)config_viewDeleted, self);
	g_signal_connect(config, "site-deleted", (GCallback)config_siteDeleted, self);

	return (GltkWidget*)gobject;
}

static gboolean
site_clicked(GltkButton* button, GltkEventClick* event, Site* siteInfo)
{
	gltk_widget_set_visible(siteInfo->views, !gltk_widget_get_visible(siteInfo->views));

	return TRUE;
}

static Site*
create_site(DarxenSiteList* list, const gchar* site)
{
	DarxenRadarSiteInfo* radarSiteInfo = darxen_radar_sites_get_site_info(site);
	
	Site* siteInfo = g_new(Site, 1);
	siteInfo->list = list;
	siteInfo->name = g_strdup(site);
	siteInfo->siteBox = gltk_vbox_new(0);
	siteInfo->views = gltk_list_new();
	g_object_set(siteInfo->views, "deletable", TRUE, NULL);
	g_signal_connect(siteInfo->views, "item-deleted", G_CALLBACK(darxen_site_list_view_deleted), list);
	siteInfo->viewMap = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)delete_view_list_item);
	g_object_ref(G_OBJECT(siteInfo->siteBox));
	g_object_ref(G_OBJECT(siteInfo->views));

	gchar* displayName = g_strdup_printf("%s, %s", radarSiteInfo->chrCity, radarSiteInfo->chrState);
	GltkWidget* siteButton = gltk_button_new(displayName);
	g_free(displayName);
	g_signal_connect(siteButton, "click-event", (GCallback)site_clicked, siteInfo);

	GltkWidget* hbox = gltk_hbox_new(0);

	gltk_box_append_widget(GLTK_BOX(hbox), gltk_label_new("  "), FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), siteInfo->views, TRUE, TRUE);

	gltk_box_append_widget(GLTK_BOX(siteInfo->siteBox), siteButton, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(siteInfo->siteBox), hbox, TRUE, TRUE);

	return siteInfo;
}

void
darxen_site_list_add_site(DarxenSiteList* list, const gchar* site)
{
	g_return_if_fail(DARXEN_IS_SITE_LIST(list));
	USING_PRIVATE(list);

	Site* siteInfo = create_site(list, site);

	GltkListItem* listItem = gltk_list_add_item(GLTK_LIST(list), siteInfo->siteBox, siteInfo);

	g_hash_table_insert(priv->siteMap, g_strdup(site), listItem);
}

static GltkWidget*
set_current_config_button(DarxenSiteList* list, GltkWidget* button)
{
	USING_PRIVATE(list);

	GltkWidget* prevConfigButton = priv->currentConfigButton;
	if (prevConfigButton)
		g_object_set(prevConfigButton, "config-mode", FALSE, NULL);

	priv->currentConfigButton = button;

	return prevConfigButton;
}

static gboolean
view_clicked(GltkButton* button, GltkEventClick* event, View* viewInfo)
{
	if (GLTK_WIDGET(button) == set_current_config_button(viewInfo->site->list, NULL))
		g_signal_emit(	G_OBJECT(viewInfo->site->list), signals[SAVE_VIEW_CONFIG], 0,
						viewInfo->site->name, viewInfo->name);
	
	g_signal_emit(G_OBJECT(viewInfo->site->list), signals[VIEW_SELECTED], 0, viewInfo->site->name, viewInfo->name);
	return FALSE;
}

static void
view_config(GltkButton* button, View* viewInfo)
{
	set_current_config_button(viewInfo->site->list, GLTK_WIDGET(button));

	g_signal_emit(G_OBJECT(viewInfo->site->list), signals[VIEW_CONFIG], 0, viewInfo->site->name, viewInfo->name);
}

static void
view_slide(GltkButton* button, gboolean dirRight, View* viewInfo)
{
	set_current_config_button(viewInfo->site->list, NULL);

	if (dirRight)
		g_signal_emit(	G_OBJECT(viewInfo->site->list), signals[SAVE_VIEW_CONFIG], 0,
						viewInfo->site->name, viewInfo->name);
	else
		g_signal_emit(	G_OBJECT(viewInfo->site->list), signals[REVERT_VIEW_CONFIG], 0,
						viewInfo->site->name, viewInfo->name);

	g_signal_emit(	G_OBJECT(viewInfo->site->list), signals[VIEW_SELECTED], 0,
					viewInfo->site->name, viewInfo->name);
}

void
darxen_site_list_add_view(DarxenSiteList* list, const gchar* site, const gchar* view)
{
	g_return_if_fail(DARXEN_IS_SITE_LIST(list));
	USING_PRIVATE(list);

	GltkListItem* listItem = (GltkListItem*)g_hash_table_lookup(priv->siteMap, site);
	g_return_if_fail(listItem);

	Site* siteInfo = (Site*)listItem->data;

	View* viewInfo = g_new(View, 1);
	viewInfo->site = siteInfo;
	viewInfo->name = g_strdup(view);
	gchar* editText = g_strdup_printf("- %s -", view);
	viewInfo->button = gltk_config_button_new(editText, view);
	g_free(editText);
	g_object_ref(G_OBJECT(viewInfo->button));
	g_signal_connect(viewInfo->button, "click-event", (GCallback)view_clicked, viewInfo);
	g_signal_connect(viewInfo->button, "slide-event", (GCallback)view_slide, viewInfo);
	g_signal_connect(viewInfo->button, "config-start", (GCallback)view_config, viewInfo);
	
	GltkListItem* viewListItem = gltk_list_add_item(GLTK_LIST(siteInfo->views), viewInfo->button, viewInfo);

	g_hash_table_insert(siteInfo->viewMap, g_strdup(view), viewListItem);
}

gchar*
darxen_site_list_free_site_data(gpointer data)
{
	Site* siteInfo = (Site*)data;
	gchar* name = g_strdup(siteInfo->name);

	darxen_config_delete_site(NULL, siteInfo->name);
	return name;
}

GQuark
darxen_site_list_error_quark()
{
	return g_quark_from_static_string("darxen-site-list-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
darxen_site_list_site_inserted(GltkList* list, GltkListItem* item, int index)
{
	USING_PRIVATE(list);

	Site* siteInfo = (Site*)item->data;

	g_hash_table_insert(priv->siteMap, g_strdup(siteInfo->name), item);

	darxen_config_add_site(NULL, siteInfo->name, index);
}

static void
darxen_site_list_site_moved(GltkList* list, GltkListItem* item, int oldIndex)
{
	Site* site = (Site*)item->data;

	int newIndex = gltk_list_get_index(list, item);
	g_debug("Moving %s from %i to %i", site->name, oldIndex, newIndex);

	darxen_config_move_site(NULL, site->name, oldIndex, newIndex);
}

static void
darxen_site_list_site_deleted(GltkList* list, GltkListItem* item)
{
	Site* site = (Site*)item->data;

	gchar* name = g_strdup(site->name);
	darxen_config_delete_site(NULL, name);
	darxen_main_view_readd_site(name);
	g_free(name);
}

static GltkWidget*
darxen_site_list_site_convert_dropped_item(GltkList* list, const gchar* targetType, GltkListItem* item)
{
	DarxenRadarSiteInfo* radarSiteInfo = (DarxenRadarSiteInfo*)item->data;

	gchar* name = g_ascii_strdown(radarSiteInfo->chrID, -1);
	Site* siteInfo = create_site(DARXEN_SITE_LIST(list), name);
	g_free(name);
	item->data = siteInfo;

	return siteInfo->siteBox;
}

static void
darxen_site_list_view_deleted(GltkList* viewList, GltkListItem* item, DarxenSiteList* siteList)
{
	g_critical("TODO delete view");
	//darxen_config_save_settings(NULL);
}

static void
config_siteDeleted			(	DarxenConfig* config,
								const gchar* site,
								DarxenSiteList* list)
{
	USING_PRIVATE(list);

	g_debug("Deleting site %s", site);

	//GltkListItem* itemSite = (GltkListItem*)g_hash_table_lookup(priv->siteMap, site);
	//g_return_if_fail(itemSite);
	//Site* siteInfo = (Site*)itemSite->data;

	g_hash_table_remove(priv->siteMap, site);
	//g_hash_table_destroy(siteInfo->viewMap);
	//g_free(siteInfo->name);
	//g_object_unref(siteInfo->siteBox);
	//g_object_unref(siteInfo->views);
	//g_free(siteInfo);
}

static void
config_viewDeleted			(	DarxenConfig* config,
								const gchar* site,
								const gchar* view,
								DarxenSiteList* list)
{
	USING_PRIVATE(list);

	g_debug("Deleting view %s/%s", site, view);

	GltkListItem* itemSite = (GltkListItem*)g_hash_table_lookup(priv->siteMap, site);
	g_return_if_fail(itemSite);
	Site* siteInfo = (Site*)itemSite->data;

	GltkListItem* itemView = (GltkListItem*)g_hash_table_lookup(siteInfo->viewMap, view);
	g_return_if_fail(itemView);
	//View* viewInfo = (View*)itemView->data;

	g_hash_table_remove(siteInfo->viewMap, view);
}

static void				
config_viewNameChanged(	DarxenConfig* config,
						const gchar* site,
						DarxenViewInfo* viewInfo,
						const gchar* oldName,
						DarxenSiteList* list)
{
	USING_PRIVATE(list);

	GltkListItem* itemSite = (GltkListItem*)g_hash_table_lookup(priv->siteMap, site);
	g_return_if_fail(itemSite);
	Site* siteInfo = (Site*)itemSite->data;

	gchar* key;
	GltkListItem* itemView;
	g_return_if_fail(g_hash_table_lookup_extended(siteInfo->viewMap, oldName, (gpointer*)&key, (gpointer*)&itemView));
	View* view = (View*)itemView->data;

	g_return_if_fail(g_hash_table_steal(siteInfo->viewMap, oldName));
	g_free(view->name);
	g_free(key);
	view->name = g_strdup(viewInfo->name);

	gchar* editText = g_strdup_printf("- %s -", viewInfo->name);
	g_object_set(G_OBJECT(view->button), "text", editText, "display-text", viewInfo->name, NULL);
	g_free(editText);

	g_hash_table_insert(siteInfo->viewMap, g_strdup(viewInfo->name), itemView);
}





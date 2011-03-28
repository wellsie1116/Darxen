/* darxenconfig.c
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

#include "darxenconfig.h"

#include <libdarxenShapefiles.h>
#include <gltk/gltkmarshal.h>

G_DEFINE_TYPE(DarxenConfig, darxen_config, G_TYPE_OBJECT)

#define USING_PRIVATE(obj) DarxenConfigPrivate* priv = DARXEN_CONFIG_GET_PRIVATE(obj)
#define DARXEN_CONFIG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_CONFIG, DarxenConfigPrivate))

enum
{
	VIEW_NAME_CHANGED,
	VIEW_UPDATED,

	LAST_SIGNAL
};

typedef struct _DarxenConfigPrivate		DarxenConfigPrivate;
struct _DarxenConfigPrivate
{
	int dummy;
};

static guint signals[LAST_SIGNAL] = {0,};
static DarxenConfig* instance = NULL;

static GObject*
darxen_config_constructor(GType type, guint paramCount, GObjectConstructParam* params)
{
	GObject* object;
	
	if (!instance)
	{
		object = G_OBJECT_CLASS(darxen_config_parent_class)->constructor(type, paramCount, params);
		instance = DARXEN_CONFIG(object);
	}
	else
	{
		object = g_object_ref(instance);
	}

	return object;
}

static void
darxen_config_class_init(DarxenConfigClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(DarxenConfigPrivate));

	signals[VIEW_NAME_CHANGED] = 
		g_signal_new(	"view-name-changed",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(DarxenConfigClass, view_name_changed),
						NULL, NULL,
						g_cclosure_user_marshal_VOID__STRING_POINTER_STRING,
						G_TYPE_NONE, 3,
						G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_STRING);
	
	signals[VIEW_UPDATED] = 
		g_signal_new(	"view-updated",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(DarxenConfigClass, view_updated),
						NULL, NULL,
						g_cclosure_user_marshal_VOID__STRING_POINTER,
						G_TYPE_NONE, 2,
						G_TYPE_STRING, G_TYPE_POINTER);

	gobject_class->constructor = darxen_config_constructor;
}

static void
darxen_config_init(DarxenConfig* self)
{
	//USING_PRIVATE(self);

	self->client = NULL;
	self->sites = NULL;
}

DarxenConfig*
darxen_config_get_instance()
{
	GObject *gobject = g_object_new(DARXEN_TYPE_CONFIG, NULL);
	//DarxenConfig* self = DARXEN_CONFIG(gobject);

	//USING_PRIVATE(self);

	return (DarxenConfig*)gobject;
}

#define ADD_SHAPEFILE(id) sf = darxen_shapefiles_clone(darxen_shapefiles_load_by_id( id )); sf->visible=TRUE; view->shapefiles = g_slist_append(view->shapefiles, sf)

GList*
darxen_config_get_sites(DarxenConfig* config)
{
	g_return_val_if_fail(DARXEN_IS_CONFIG(config), NULL);

	if (!config->sites)
	{
		DarxenSiteInfo* site;
		DarxenViewInfo* view;
		DarxenShapefile* sf;

		//KLOT
		site = g_new(DarxenSiteInfo, 1);
		site->name = "klot";
		site->views = NULL;

		view = g_new(DarxenViewInfo, 1);
		view->name = g_strdup("Archived Reflectivity (0.5)");
		view->productCode = g_strdup("N0R");
		view->sourceType = DARXEN_VIEW_SOURCE_ARCHIVE;
		view->source.archive.startId = g_strdup("201102101939");
		view->source.archive.endId = g_strdup("201102102304");
		view->shapefiles = NULL;
		ADD_SHAPEFILE("Counties");
		ADD_SHAPEFILE("States");
		ADD_SHAPEFILE("Lakes");
		view->smoothing = FALSE;
		site->views = g_list_append(site->views, view);

		view = g_new(DarxenViewInfo, 1);
		view->name = g_strdup("Archived Reflectivity (1.0)");
		view->productCode = g_strdup("N1R");
		view->sourceType = DARXEN_VIEW_SOURCE_ARCHIVE;
		view->source.archive.startId = g_strdup("201101121416");
		view->source.archive.endId = g_strdup("201101121456");
		view->shapefiles = NULL;
		ADD_SHAPEFILE("Counties");
		ADD_SHAPEFILE("States");
		view->smoothing = FALSE;
		site->views = g_list_append(site->views, view);

		config->sites = g_list_append(config->sites, site);

		//KIND
		site = g_new(DarxenSiteInfo, 1);
		site->name = "kind";
		site->views = NULL;

		view = g_new(DarxenViewInfo, 1);
		view->name = g_strdup("Live Reflectivity (0.5)");
		view->productCode = g_strdup("N0R");
		view->sourceType = DARXEN_VIEW_SOURCE_LIVE;
		view->shapefiles = NULL;
		ADD_SHAPEFILE("Counties");
		ADD_SHAPEFILE("States");
		ADD_SHAPEFILE("Lakes");
		ADD_SHAPEFILE("Rivers");
		ADD_SHAPEFILE("Sites");
		view->smoothing = FALSE;
		site->views = g_list_append(site->views, view);
		
		view = g_new(DarxenViewInfo, 1);
		view->name = g_strdup("Thunderstorm (3-18)");
		view->productCode = g_strdup("N0R");
		view->sourceType = DARXEN_VIEW_SOURCE_ARCHIVE;
		view->source.archive.startId = g_strdup("201103180000");
		view->source.archive.endId = g_strdup("201103180250");
		view->shapefiles = NULL;
		ADD_SHAPEFILE("Counties");
		ADD_SHAPEFILE("States");
		view->smoothing = FALSE;
		site->views = g_list_append(site->views, view);

		view = g_new(DarxenViewInfo, 1);
		view->name = g_strdup("Archived Reflectivity (1.0)");
		view->productCode = g_strdup("N1R");
		view->sourceType = DARXEN_VIEW_SOURCE_ARCHIVE;
		view->source.archive.startId = g_strdup("201101121417");
		view->source.archive.endId = g_strdup("201101121456");
		view->shapefiles = NULL;
		ADD_SHAPEFILE("States");
		view->smoothing = FALSE;
		site->views = g_list_append(site->views, view);

		config->sites = g_list_append(config->sites, site);
	}

	return config->sites;
}

void
darxen_config_set_client(DarxenConfig* config, DarxenRestfulClient* newClient)
{
	g_return_if_fail(DARXEN_IS_CONFIG(config));
	g_return_if_fail(DARXEN_IS_RESTFUL_CLIENT(newClient));

	config->client = newClient;
}

DarxenRestfulClient*
darxen_config_get_client(DarxenConfig* config)
{
	g_return_val_if_fail(DARXEN_IS_CONFIG(config), NULL);

	return config->client;
}

gboolean
darxen_config_rename_view(	DarxenConfig* config,
							const gchar* site,
							DarxenViewInfo* viewInfo,
							const gchar* newName)
{
	g_return_val_if_fail(DARXEN_IS_CONFIG(config), FALSE);

	//find the site
	GList* pSites = config->sites;
	DarxenSiteInfo* siteInfo = NULL;
	while (pSites && !siteInfo)
	{
		siteInfo = (DarxenSiteInfo*)pSites->data;
				
		if (g_strcmp0(siteInfo->name, site))
			siteInfo = NULL;
	
		pSites = pSites->next;
	}
	g_return_val_if_fail(siteInfo, FALSE);

	//check for duplicates
	GList* pViews = siteInfo->views;
	while (pViews)
	{
		DarxenViewInfo* view = (DarxenViewInfo*)pViews->data;
	
		g_return_val_if_fail(g_strcmp0(view->name, newName), FALSE);
	
		pViews = pViews->next;
	}

	gchar* oldName = viewInfo->name;
	viewInfo->name = g_strdup(newName);

	g_signal_emit(config, signals[VIEW_NAME_CHANGED], 0, site, viewInfo, oldName);
	g_free(oldName);

	return TRUE;
}

void
darxen_config_view_updated(	DarxenConfig* config,
							const gchar* site,
							DarxenViewInfo* viewInfo)
{
	g_return_if_fail(DARXEN_IS_CONFIG(config));
	
	g_signal_emit(config, signals[VIEW_NAME_CHANGED], 0, site, viewInfo);
}


GQuark
darxen_config_error_quark()
{
	return g_quark_from_static_string("darxen-config-error-quark");
}

/*********************
 * Private Functions *
 *********************/


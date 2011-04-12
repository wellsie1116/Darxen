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
#include <libdarxenFileSupport.h>
#include <gltk/gltkmarshal.h>

#include <json-glib/json-glib.h>

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
	gboolean settingsLoaded;
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
						g_cclosure_user_marshal_VOID__STRING_STRING_POINTER,
						G_TYPE_NONE, 3,
						G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

	gobject_class->constructor = darxen_config_constructor;
}

static void
darxen_config_init(DarxenConfig* self)
{
	USING_PRIVATE(self);

	self->client = NULL;
	self->sites = NULL;
	
	priv->settingsLoaded = FALSE;
}

DarxenConfig*
darxen_config_get_instance()
{
	GObject *gobject = g_object_new(DARXEN_TYPE_CONFIG, NULL);
	//DarxenConfig* self = DARXEN_CONFIG(gobject);

	//USING_PRIVATE(self);

	return (DarxenConfig*)gobject;
}

const gchar* settings_path() G_GNUC_CONST;
const gchar* settings_path()
{
	static gchar* path = NULL;
	if (!path)
		path = g_build_filename(darxen_file_support_get_app_path(), "settings.json", NULL);

	return path;
}

#define ADD_SHAPEFILE(id) sf = darxen_shapefiles_load_by_id( id ); sf->visible=TRUE; view->shapefiles = g_slist_append(view->shapefiles, sf)

void
darxen_config_load_settings(DarxenConfig* config)
{
	GError* error = NULL;
	USING_PRIVATE(config);

	if (priv->settingsLoaded)
		return;

	JsonParser* parser = json_parser_new();
	gboolean res = json_parser_load_from_file(parser, settings_path(), &error);

	if (!res)
	{
		g_object_unref(parser);
		g_message("Failed to load configuration, loading defaults.  Details: %s", error->message);
		
		//load some test data
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

		priv->settingsLoaded = TRUE;

		darxen_config_save_settings(config);
		return;
	}


	JsonNode* rootNode = json_parser_get_root(parser);
	JsonObject* root = json_node_get_object(rootNode);

	JsonArray* sites = json_object_get_array_member(root, "sites");
	int i;
	for (i = 0; i < json_array_get_length(sites); i++)
	{
		JsonObject* site = json_array_get_object_element(sites, i);

		DarxenSiteInfo* siteInfo = g_new(DarxenSiteInfo, 1);
		siteInfo->name = g_strdup(json_object_get_string_member(site, "name"));
		siteInfo->views = NULL;

		JsonArray* views = json_object_get_array_member(site, "views");
		int j;
		for (j = 0; j < json_array_get_length(views); j++)
		{
			JsonObject* view = json_array_get_object_element(views, j);

			DarxenViewInfo* viewInfo = g_new(DarxenViewInfo, 1);
			viewInfo->name = g_strdup(json_object_get_string_member(view, "name"));
			viewInfo->productCode = g_strdup(json_object_get_string_member(view, "productCode"));
			viewInfo->smoothing = json_object_get_boolean_member(view, "smoothing");
			const gchar* sourceType = json_object_get_string_member(view, "sourceType");
			if (!g_strcmp0(sourceType, "archive"))
				viewInfo->sourceType = DARXEN_VIEW_SOURCE_ARCHIVE;
			else if (!g_strcmp0(sourceType, "live"))
				viewInfo->sourceType = DARXEN_VIEW_SOURCE_LIVE;
			else
				g_assert_not_reached();

			JsonObject* sourceParams = json_object_get_object_member(view, "sourceParams");
			if (viewInfo->sourceType == DARXEN_VIEW_SOURCE_ARCHIVE)
			{
				viewInfo->source.archive.startId = g_strdup(json_object_get_string_member(sourceParams,
							"startId"));
				viewInfo->source.archive.endId = g_strdup(json_object_get_string_member(sourceParams,
							"endId"));
			}
			else
			{
				//no settings
			}

			viewInfo->shapefiles = NULL;
			JsonArray* shapefiles = json_object_get_array_member(view, "shapefiles");
			int k;
			for (k = 0; k < json_array_get_length(shapefiles); k++)
			{
				JsonObject* shapefile = json_array_get_object_element(shapefiles, k);

				const gchar* id = json_object_get_string_member(shapefile, "id");
				gboolean visible = json_object_get_boolean_member(shapefile, "visible");

				DarxenShapefile* shapefileInfo = darxen_shapefiles_load_by_id(id);
				shapefileInfo->visible = visible;
				viewInfo->shapefiles = g_slist_append(viewInfo->shapefiles, shapefileInfo);
			}
			siteInfo->views = g_list_append(siteInfo->views, viewInfo);

		}
		config->sites = g_list_append(config->sites, siteInfo);

	}
}

void
darxen_config_save_settings(DarxenConfig* config)
{
	GError* error = NULL;
	JsonObject* root = json_object_new();

	JsonArray* sites = json_array_new();
	GList* pSites = config->sites;
	while (pSites)
	{
		DarxenSiteInfo* siteInfo = (DarxenSiteInfo*)pSites->data;
	
		JsonObject* site = json_object_new();
		{
			json_object_set_string_member(site, "name", siteInfo->name);

			JsonArray* views = json_array_new();
			GList* pViews = siteInfo->views;
			while (pViews)
			{
				DarxenViewInfo* viewInfo = (DarxenViewInfo*)pViews->data;
			
				JsonObject* view = json_object_new();
				{
					json_object_set_string_member(view, "name", viewInfo->name);
					json_object_set_string_member(view, "productCode", viewInfo->productCode);
					json_object_set_boolean_member(view, "smoothing", viewInfo->smoothing);
					json_object_set_string_member(view, "sourceType",
							viewInfo->sourceType == DARXEN_VIEW_SOURCE_ARCHIVE ? "archive" : "live");
					JsonObject* sourceParams = json_object_new();
					if (viewInfo->sourceType == DARXEN_VIEW_SOURCE_ARCHIVE)
					{
						json_object_set_string_member(sourceParams, "startId",
								viewInfo->source.archive.startId);
						json_object_set_string_member(sourceParams, "endId",
								viewInfo->source.archive.endId);
					}
					else
					{
						//no settings
					}
					json_object_set_object_member(view, "sourceParams", sourceParams);

					JsonArray* shapefiles = json_array_new();
					GSList* pShapefiles = viewInfo->shapefiles;
					while (pShapefiles)
					{
						DarxenShapefile* shapefileInfo = (DarxenShapefile*)pShapefiles->data;
					
						JsonObject* shapefile = json_object_new();
						{
							json_object_set_string_member(shapefile, "id", shapefileInfo->name);
							json_object_set_boolean_member(shapefile, "visible", shapefileInfo->visible);
						}
						json_array_add_object_element(shapefiles, shapefile);
					
						pShapefiles = pShapefiles->next;
					}
					json_object_set_array_member(view, "shapefiles", shapefiles);
				}

				json_array_add_object_element(views, view);
			
				pViews = pViews->next;
			}
			json_object_set_array_member(site, "views", views);
		}
					
		json_array_add_object_element(sites, site);
	
		pSites = pSites->next;
	}

	json_object_set_array_member(root, "sites", sites);

	JsonNode* rootNode = json_node_new(JSON_NODE_OBJECT);
	json_node_take_object(rootNode, root);

	JsonGenerator* generator = json_generator_new();
	g_object_set(generator, "pretty", TRUE, NULL);
	json_generator_set_root(generator, rootNode);
	json_node_free(rootNode);

	gboolean res = json_generator_to_file(generator, settings_path(), &error);
	g_object_unref(generator);

	if (!res)
		g_critical("Failed to save settings: %s", error->message);
}



GList*
darxen_config_get_sites(DarxenConfig* config)
{
	g_return_val_if_fail(DARXEN_IS_CONFIG(config), NULL);

	darxen_config_load_settings(config);

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
							const DarxenViewInfo* viewInfo,
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
	DarxenViewInfo* view;
	while (pViews)
	{
		view = (DarxenViewInfo*)pViews->data;
	
		g_return_val_if_fail(g_strcmp0(view->name, newName), FALSE);
	
		pViews = pViews->next;
	}

	//find our copy of the viewInfo
	pViews = siteInfo->views;
	while (pViews)
	{
		view = (DarxenViewInfo*)pViews->data;
	
		if (!g_strcmp0(view->name, viewInfo->name))
			break;
	
		pViews = pViews->next;
	}
	g_assert(pViews && view);

	gchar* oldName = view->name;
	view->name = g_strdup(newName);
	g_signal_emit(config, signals[VIEW_NAME_CHANGED], 0, site, view, oldName);
	g_free(oldName);
	
	darxen_config_save_settings(config);

	return TRUE;
}

void
darxen_config_view_updated(	DarxenConfig* config,
							const gchar* site,
							const gchar* viewName,
							DarxenViewInfo* viewInfo)
{
	g_return_if_fail(DARXEN_IS_CONFIG(config));
	
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
	g_assert(siteInfo);

	//find the view info
	DarxenViewInfo* view;
	GList* pViews = siteInfo->views;
	while (pViews)
	{
		view = (DarxenViewInfo*)pViews->data;
	
		if (!g_strcmp0(view->name, viewName))
			break;
	
		pViews = pViews->next;
	}
	g_assert(pViews && view);

	g_signal_emit(config, signals[VIEW_UPDATED], 0, site, viewName, viewInfo);

	//all views should have swapped references, free the old one, copy the new one
	darxen_view_info_free(view);
	pViews->data = darxen_view_info_copy(viewInfo);

	darxen_config_save_settings(config);
}

DarxenViewInfo*
darxen_view_info_copy(const DarxenViewInfo* viewInfo)
{
	DarxenViewInfo* newInfo = g_new(DarxenViewInfo, 1);

	*newInfo = *viewInfo;

	newInfo->name = g_strdup(newInfo->name);
	newInfo->productCode = g_strdup(newInfo->productCode);

	switch (newInfo->sourceType)
	{
		case DARXEN_VIEW_SOURCE_ARCHIVE:
			newInfo->source.archive.startId = g_strdup(newInfo->source.archive.startId);
			newInfo->source.archive.endId = g_strdup(newInfo->source.archive.endId);
			break;
		case DARXEN_VIEW_SOURCE_LIVE:
			break;
	}

	newInfo->shapefiles = g_slist_copy(newInfo->shapefiles);
	GSList* pShapefiles;
	for (pShapefiles = newInfo->shapefiles; pShapefiles; pShapefiles = pShapefiles->next)
		pShapefiles->data = darxen_shapefiles_clone((DarxenShapefile*)pShapefiles->data);

	return newInfo;
}

void
darxen_view_info_free(DarxenViewInfo* viewInfo)
{
	g_free(viewInfo->name);
	g_free(viewInfo->productCode);

	switch (viewInfo->sourceType)
	{
		case DARXEN_VIEW_SOURCE_ARCHIVE:
			g_free(viewInfo->source.archive.startId);
			g_free(viewInfo->source.archive.endId);
			break;
		case DARXEN_VIEW_SOURCE_LIVE:
			break;
	}

	GSList* pShapefiles;
	for (pShapefiles = viewInfo->shapefiles; pShapefiles; pShapefiles = pShapefiles->next)
		darxen_shapefiles_free_shapefile((DarxenShapefile*)pShapefiles->data);
	g_slist_free(viewInfo->shapefiles);
}


GQuark
darxen_config_error_quark()
{
	return g_quark_from_static_string("darxen-config-error-quark");
}

/*********************
 * Private Functions *
 *********************/


/* darxenradarviewer.c
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

#include "darxenradarviewer.h"

#include "DarxenParserLevel3.h"
#include <darxenrenderer.h>

G_DEFINE_TYPE(DarxenRadarViewer, darxen_radar_viewer, GLTK_TYPE_WIDGET)

#define USING_PRIVATE(obj) DarxenRadarViewerPrivate* priv = DARXEN_RADAR_VIEWER_GET_PRIVATE(obj)
#define DARXEN_RADAR_VIEWER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_RADAR_VIEWER, DarxenRadarViewerPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _DarxenRadarViewerPrivate	DarxenRadarViewerPrivate;
typedef struct _RenderData					RenderData;
struct _DarxenRadarViewerPrivate
{
	gchar* site;
	DarxenViewInfo* viewInfo;

	DarxenRenderer* renderer;
	GList* data; //RenderData
};

struct _RenderData
{
	gchar* id;
	ProductsLevel3Data* data;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void darxen_radar_viewer_dispose(GObject* gobject);
static void darxen_radar_viewer_finalize(GObject* gobject);

static void	darxen_radar_viewer_render(GltkWidget* widget);

static void
darxen_radar_viewer_class_init(DarxenRadarViewerClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(DarxenRadarViewerPrivate));
	
	gobject_class->dispose = darxen_radar_viewer_dispose;
	gobject_class->finalize = darxen_radar_viewer_finalize;

	gltkwidget_class->render = darxen_radar_viewer_render;
}

static void
darxen_radar_viewer_init(DarxenRadarViewer* self)
{
	USING_PRIVATE(self);

	priv->viewInfo = NULL;

	priv->renderer = NULL;
	priv->data = NULL;
}

static void
darxen_radar_viewer_dispose(GObject* gobject)
{
	DarxenRadarViewer* self = DARXEN_RADAR_VIEWER(gobject);
	USING_PRIVATE(self);

	//free and release references

	if (priv->renderer)
	{
		g_object_unref(G_OBJECT(priv->renderer));
		priv->renderer = NULL;
	}

	G_OBJECT_CLASS(darxen_radar_viewer_parent_class)->dispose(gobject);
}

static void
darxen_radar_viewer_finalize(GObject* gobject)
{
	DarxenRadarViewer* self = DARXEN_RADAR_VIEWER(gobject);
	USING_PRIVATE(self);

	g_free(priv->site);

	G_OBJECT_CLASS(darxen_radar_viewer_parent_class)->finalize(gobject);
}

DarxenRadarViewer*
darxen_radar_viewer_new(const gchar* site, DarxenViewInfo* viewInfo)
{
	GObject *gobject = g_object_new(DARXEN_TYPE_RADAR_VIEWER, NULL);
	DarxenRadarViewer* self = DARXEN_RADAR_VIEWER(gobject);

	USING_PRIVATE(self);

	priv->site = g_strdup(site);
	priv->viewInfo = viewInfo;

	priv->renderer = darxen_renderer_new(priv->site, priv->viewInfo->productCode);

	DarxenRestfulClient* client = darxen_config_get_client();
	switch (viewInfo->sourceType)
	{
		case DARXEN_VIEW_SOURCE_ARCHIVE:
		{
			GError* error = NULL;

			//run the search
			int searchId;
			int searchCount;
			if (!darxen_restful_client_search_data(	client, site, viewInfo->productCode, 
													viewInfo->source.archive.startId, 
													viewInfo->source.archive.endId, 
													&searchId, &searchCount, &error))
			{
				g_error("Search failed for %s/%s in view %s", site, viewInfo->productCode, viewInfo->name);
			}

			//get the search results
			gchar** ids;
			if (!(ids = darxen_restful_client_read_search(client, searchId, 0, searchCount, &error)))
			{
				g_error("Failed to retrieve search records for %s/%s in view %s", site, viewInfo->productCode, viewInfo->name);
			}

			//read the search records
			int i;
			for (i = 0; i < searchCount; i++)
			{
				size_t len;
				char* data = darxen_restful_client_read_data(client, site, viewInfo->productCode, ids[i], &len, &error);

				if (!data)
				{
					g_warning("Failed to download data %s/%s with id %s", site, viewInfo->productCode, ids[i]);
					continue;
				}

				FILE* f = tmpfile();
				fwrite(data, 1, len, f);
				fseek(f, 0, SEEK_SET);
				ProductsLevel3Data* parsed;
				if (!(parsed = parser_lvl3_parse_file(f)))
				{
					g_critical("Failed to parse level 3 data");
					free(data);
					continue;
				}
				fclose(f);
				printf("Header: %s\n", parsed->chrWmoHeader);fflush(stdout);
				RenderData* renderData = g_new(RenderData, 1);
				renderData->id = g_strdup(ids[i]);
				renderData->data = parsed;

				priv->data = g_list_append(priv->data, renderData);
				
				free(data);
			}
			g_strfreev(ids);

			//free the search
			if (!darxen_restful_client_free_search(client, searchId, &error))
			{
				g_error("Failed to free search for %s/%s in view %s", site, viewInfo->productCode, viewInfo->name);
			}

			darxen_renderer_set_data(priv->renderer, ((RenderData*)priv->data->data)->data);

		} break;
	}

	return (DarxenRadarViewer*)gobject;
}


GQuark
darxen_radar_viewer_error_quark()
{
	return g_quark_from_static_string("darxen-radar-viewer-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
darxen_radar_viewer_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);

	if (!priv->data)
		return;

	GltkAllocation allocation = gltk_widget_get_allocation(widget);

	glPushMatrix();
	{
		glTranslatef(allocation.width / 2, allocation.height / 2, 0.0f);
		darxen_renderer_render(priv->renderer);
	}
	glPopMatrix();
}



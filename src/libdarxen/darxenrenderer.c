/* darxenrenderer.c

   Copyright (C) 2011 Kevin Wells <kevin@darxen.org>

   This file is part of darxen.

   darxen is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   darxen is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with darxen.  If not, see <http://www.gnu.org/licenses/>.
*/

#define G_LOG_DOMAIN "DarxenRenderer"
#include "glib/gmessages.h"

#include "darxenrenderer.h"

#include "darxenpalettes.h"

//#include "DarxenConversions.h"
//#include "DarxenRendererShared.h"

#include "libdarxenRadarSites.h"
#include "libdarxenShapefiles.h"
#include "libdarxenShapefileRenderer.h"
#include "libdarxenRenderingCommon.h"

#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>

#ifndef GL_BGRA_EXT
#ifdef GL_BGRA
#define GL_BGRA_EXT GL_BGRA
#else
#define GL_BGRA_EXT 0x80E1
#endif
#endif

#define DEG_TO_RAD(deg) ((deg/180.0)*G_PI)
#define RAD_TO_DEG(rad) ((rad/G_PI)*180.0)

#define VIEW_AREA 5.0

typedef struct _DarxenRendererPrivate	DarxenRendererPrivate;
typedef struct _ShapefileInfo			ShapefileInfo;
typedef enum _OverlayType				OverlayType;
typedef enum _OverlayLocation			OverlayLocation;
typedef struct _OverlayInfo				OverlayInfo;
typedef struct _ProductInfo				ProductInfo;

enum _ShapefileParts
{
	SHAPEFILE_PART_POLYGONS,
	SHAPEFILE_PART_LINES,
	SHAPEFILE_PART_POINTS,
	SHAPEFILE_PART_TEXT_LABELS,

	SHAPEFILE_PART_COUNT
};

struct _ShapefileInfo
{
	DarxenShapefile* shapefile;
	GLuint** dspDataLevels;
};

enum _OverlayType
{
	OVERLAY_TYPE_LEGEND,
	OVERLAY_TYPE_VCP,
	OVERLAY_TYPE_MX,
	OVERLAY_TYPE_DATETIME,

	OVERLAY_TYPE_COUNT
};

enum _OverlayLocation
{
	OVERLAY_LOCATION_TOP_LEFT,
	OVERLAY_LOCATION_TOP_RIGHT,
	OVERLAY_LOCATION_BOTTOM_LEFT,
	OVERLAY_LOCATION_BOTTOM_RIGHT
};       

struct _OverlayInfo
{
	OverlayType type;
	OverlayLocation location;
};

struct _ProductInfo
{
	gchar* productCode;
	gboolean smoothing;
	//GSList* shapefiles; /* ShapefileInfo */
};

struct _DarxenRendererPrivate
{
	ProductsLevel3Data* objData;
	gchar* siteName;

	int width;
	int height;

	gboolean hasSizeChanged;
	gboolean hasDataChanged;

	float fltRadiusBase;
	float fltRadius;

	GLuint dspProductsBase;

	ProductInfo product;
	//GSList* overlays; /* OverlayInfo */

	DarxenGLFontCache* fonts;
};

/* Private Functions */
static void darxen_renderer_resize(DarxenRenderer *renderer);
static void darxen_renderer_data_changed(DarxenRenderer *renderer);
static void darxen_renderer_render_internal(DarxenRenderer *renderer);
static void darxen_renderer_render_underlay(DarxenRenderer *renderer);
static void darxen_renderer_render_overlay(DarxenRenderer *renderer);
static void darxen_renderer_shared_render_overlay_legend(DarxenRenderer *renderer);
static void darxen_renderer_render_radial_data(DarxenRenderer *renderer, ProductsLevel3RadialDataPacket *objData);

G_DEFINE_TYPE(DarxenRenderer, darxen_renderer, G_TYPE_OBJECT)

static void darxen_renderer_finalize(GObject* gobject);

#define USING_PRIVATE(obj) DarxenRendererPrivate* priv = DARXEN_RENDERER_GET_PRIVATE(obj);
#define DARXEN_RENDERER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_RENDERER, DarxenRendererPrivate))

static void
darxen_renderer_class_init(DarxenRendererClass *klass)
{
	g_type_class_add_private(klass, sizeof(DarxenRendererPrivate));

  	klass->parent_class.finalize = darxen_renderer_finalize;
}

static void
darxen_renderer_init(DarxenRenderer *renderer)
{
	USING_PRIVATE(renderer);

	renderer->scale				= 1.0f;
	renderer->offset.x			= 0.0f;
	renderer->offset.y			= 0.0f;

	priv->objData				= NULL;
	priv->siteName				= NULL;

	priv->width					= 0;
	priv->height				= 0;

	priv->hasSizeChanged		= TRUE;
	priv->hasDataChanged		= TRUE;

	priv->fltRadiusBase			= 0.0f;
	priv->fltRadius				= 0.0f;
	priv->dspProductsBase		= 0;

	//priv->overlays				= NULL;

	priv->fonts					= NULL;

	priv->product.productCode	= NULL;
	priv->product.smoothing		= FALSE;
	//priv->product.shapefiles	= NULL;


}

static void
darxen_renderer_finalize(GObject* gobject)
{
	DarxenRenderer* self = DARXEN_RENDERER(gobject);

	USING_PRIVATE(self);

	darxen_rendering_common_font_cache_free(priv->fonts);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(darxen_renderer_parent_class)->finalize(gobject);
}

/********************
 * Public Functions *
 ********************/

DarxenRenderer*
darxen_renderer_new(const gchar* siteName, const gchar* productCode)
{
	GObject* gobject = g_object_new(DARXEN_TYPE_RENDERER, NULL);
	DarxenRenderer* renderer = DARXEN_RENDERER(gobject);

	USING_PRIVATE(renderer);

	priv->siteName = g_strdup(siteName);

	priv->product.productCode = g_strdup(productCode);

	priv->fonts = darxen_rendering_common_font_cache_new();

	// priv->products = NULL;
	// GList* pProducts = viewSettings->products;
	// while (pProducts)
	// {
	// 	SettingsViewProduct* product = (SettingsViewProduct*)pProducts->data;
	// 	ProductInfo* productInfo = (ProductInfo*)malloc(sizeof(ProductInfo));
	// 	priv->products = g_slist_append(priv->products, productInfo);

	// 	productInfo->product = product;

	// 	productInfo->shapefiles = NULL;
	// 	GList* pShapefiles = product->shapefiles;
	// 	while (pShapefiles)
	// 	{
	// 		DarxenShapefile* shapefile = (DarxenShapefile*)pShapefiles->data;
	// 		ShapefileInfo* shapefileInfo = (ShapefileInfo*)malloc(sizeof(ShapefileInfo));
	// 		productInfo->shapefiles = g_slist_append(productInfo->shapefiles, shapefileInfo);

	// 		shapefileInfo->shapefile = shapefile;

	// 		int shapefileIndex;
	// 		int shapefileCount = (shapefile->dataLevels) ? g_slist_length(shapefile->dataLevels->levels) : 1;

	// 		shapefileInfo->dspDataLevels = (GLuint**)calloc(shapefileCount, sizeof(GLuint*));

	// 		for (shapefileIndex = 0; shapefileIndex < shapefileCount; shapefileIndex++)
	// 		{
	// 			int partIndex;
	// 			shapefileInfo->dspDataLevels[shapefileIndex] = (GLuint*)calloc(SHAPEFILE_PART_COUNT, sizeof(GLuint));
	// 			for (partIndex = 0; partIndex < SHAPEFILE_PART_COUNT; partIndex++)
	// 				shapefileInfo->dspDataLevels[shapefileIndex][partIndex] = 0;
	// 		}

	// 		pShapefiles = pShapefiles->next;
	// 	}

	// 	pProducts = pProducts->next;
	// }

	return renderer;
}

ProductsLevel3Data*
darxen_renderer_get_data(DarxenRenderer *renderer)
{
	g_return_val_if_fail(DARXEN_IS_RENDERER(renderer), NULL);

	USING_PRIVATE(renderer);

	return priv->objData;
}

/* Set */
void
darxen_renderer_set_data(DarxenRenderer *renderer, ProductsLevel3Data *objData)
{
	g_return_if_fail(DARXEN_IS_RENDERER(renderer));

	USING_PRIVATE(renderer);

	priv->objData = objData;

	darxen_renderer_data_changed(renderer);
}

void
darxen_renderer_set_size(DarxenRenderer *renderer, int width, int height)
{
	g_return_if_fail(DARXEN_IS_RENDERER(renderer));

	USING_PRIVATE(renderer);

	priv->width = width;
	priv->height = height;

	darxen_renderer_resize(renderer);
}

/* Utility */

void
darxen_renderer_render(DarxenRenderer *renderer)
{
	GTimer *timer;

	g_return_if_fail(DARXEN_IS_RENDERER(renderer));

	timer = g_timer_new();
	g_timer_start(timer);

	darxen_renderer_render_internal(renderer);

	g_timer_stop(timer);
	g_debug("Render Time: %f", g_timer_elapsed(timer, NULL));
	g_timer_destroy(timer);
}

/*********************
 * Private Functions *
 *********************/

static void darxen_renderer_resize(DarxenRenderer *renderer)
{
	USING_PRIVATE(renderer);

	priv->hasSizeChanged = TRUE;
}

static void
darxen_renderer_data_changed(DarxenRenderer *renderer)
{
	USING_PRIVATE(renderer);

	priv->hasDataChanged = TRUE;
}

static void
darxen_renderer_render_internal(DarxenRenderer *renderer)
{
	float fltRangeBins;

	USING_PRIVATE(renderer);

	if (!priv->objData)
	{
		g_critical("No data set");
		return;
	}

	fltRangeBins = priv->objData->objDescription.objSymbologyBlock->objPackets->objRadialData.intNumRangeBins;

	//NOTE: Units are in km
	glScalef(renderer->scale, renderer->scale, 1.0);
	glTranslatef(renderer->offset.x, renderer->offset.y, 0.0f);

	darxen_renderer_render_underlay(renderer);

	if (!priv->hasDataChanged && priv->dspProductsBase)
	{
		glCallList(priv->dspProductsBase);
	}
	else
	{
		if (priv->dspProductsBase)
			glDeleteLists(priv->dspProductsBase, 1);
		priv->dspProductsBase = glGenLists(1);
		glNewList(priv->dspProductsBase, GL_COMPILE_AND_EXECUTE);
		{
			darxen_renderer_render_radial_data(renderer, &priv->objData->objDescription.objSymbologyBlock->objPackets[0].objRadialData);
		}
		glEndList();
		priv->hasDataChanged = FALSE;
	}

	//TODO: clip here
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_LOOP);
	{
		float latCenter = priv->objData->objDescription.fltLat;
		float lonCenter = priv->objData->objDescription.fltLon;
		glVertex2fv(darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, latCenter + VIEW_AREA, lonCenter + VIEW_AREA));
		glVertex2fv(darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, latCenter - VIEW_AREA, lonCenter + VIEW_AREA));
		glVertex2fv(darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, latCenter - VIEW_AREA, lonCenter - VIEW_AREA));
		glVertex2fv(darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, latCenter + VIEW_AREA, lonCenter - VIEW_AREA));
	}
	glEnd();

	darxen_renderer_render_overlay(renderer);
}

static void
darxen_renderer_render_underlay(DarxenRenderer *renderer)
{
	USING_PRIVATE(renderer);

	// SettingsSite* site = priv->radarSite;
	// DarxenRadarSiteInfo* siteInfo = darxen_radar_sites_get_site_info(site->name);

	// SettingsRenderer* renderSettings = &settings_get_main()->renderer;
	// float fltScale = renderSettings->fltScale;

	// GSList* pShapefileInfos = ((ProductInfo*)priv->products->data)->shapefiles;
	// while (pShapefileInfos)
	// {
	// 	ShapefileInfo* shapefileInfo = (ShapefileInfo*)pShapefileInfos->data;
	// 	DarxenShapefile* shapefile = shapefileInfo->shapefile;

	// 	if (shapefile->visible)
	// 	{
	// 		int dataLevel = 0;
	// 		if (shapefile->dataLevels)
	// 		{
	// 			GSList* pDataLevels = shapefile->dataLevels->levels;

	// 			while (pDataLevels)
	// 			{
	// 				DarxenShapefileDataLevel* level = (DarxenShapefileDataLevel*)pDataLevels->data;

	// 				if (level->scale > fltScale)
	// 					break;

	// 				dataLevel++;

	// 				pDataLevels = pDataLevels->next;
	// 			}
	// 			dataLevel--;
	// 			if (dataLevel < 0)
	// 			{
	// 				g_warning("Data level does not fit with any specified ranges, defaulting to 0");
	// 				dataLevel = 0;
	// 			}
	// 		}
	// 		while (dataLevel >= 0)
	// 		{
	// 			//FIXME: site location should be taken from the radar data itself?
	// 			//FIXME: shouldn't hardcode radius?
	// 			char* pathShp = darxen_shapefiles_filter_shp(shapefile, dataLevel, siteInfo->chrID, siteInfo->fltLatitude, siteInfo->fltLongitude, 5.0);

	// 			if (shapefile->polygons && shapefile->polygons->visible)
	// 			{
	// 				glColor3fv(shapefile->polygons->color);
	// 				if (!shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POLYGONS])
	// 				{
	// 					int list = glGenLists(1);

	// 					glNewList(list, GL_COMPILE_AND_EXECUTE);
	// 					{
	// 						darxen_shapefile_renderer_render_polygons(shapefile->polygons, pathShp, siteInfo, FALSE);
	// 					}
	// 					glEndList();

	// 					shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POLYGONS] = list;
	// 				}
	// 				else
	// 				{
	// 					glCallList(shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POLYGONS]);
	// 				}
	// 			}
	// 			if (shapefile->lines && shapefile->lines->visible)
	// 			{
	// 				glColor3fv(shapefile->lines->color);
	// 				if (!shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_LINES])
	// 				{
	// 					int list = glGenLists(1);

	// 					glNewList(list, GL_COMPILE_AND_EXECUTE);
	// 					{
	// 						darxen_shapefile_renderer_render_lines(shapefile->lines, pathShp, siteInfo, FALSE);
	// 					}
	// 					glEndList();

	// 					shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_LINES] = list;
	// 				}
	// 				else
	// 				{
	// 					glCallList(shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_LINES]);
	// 				}
	// 			}
	// 			if (shapefile->points && shapefile->points->visible)
	// 			{
	// 				glColor3fv(shapefile->points->color);
	// 				if (!shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POINTS])
	// 				{
	// 					int list = glGenLists(1);

	// 					glNewList(list, GL_COMPILE_AND_EXECUTE);
	// 					{
	// 						darxen_shapefile_renderer_render_points(shapefile->points, pathShp, siteInfo, FALSE);
	// 					}
	// 					glEndList();

	// 					shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POINTS] = list;
	// 				}
	// 				else
	// 				{
	// 					glCallList(shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POINTS]);
	// 				}
	// 			}
	// 			if (shapefile->textLabels && shapefile->textLabels->visible)
	// 			{
	// 				glColor3fv(shapefile->textLabels->color);
	// 				if (!shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_TEXT_LABELS])
	// 				{
	// 					int list = glGenLists(1);

	// 					gchar* fontname = g_strdup_printf("courier new %.1f", shapefile->textLabels->fontSize);
	// 					DarxenGLFont* font = darxen_rendering_common_font_cache_get_font(priv->fonts, fontname);
	// 					g_free(fontname);

	// 					glNewList(list, GL_COMPILE_AND_EXECUTE);
	// 					{
	// 						darxen_shapefile_renderer_render_text_labels(shapefile->textLabels, pathShp, siteInfo, font, FALSE);
	// 					}
	// 					glEndList();

	// 					shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_TEXT_LABELS] = list;
	// 				}
	// 				else
	// 				{
	// 					glCallList(shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_TEXT_LABELS]);
	// 				}
	// 			}

	// 			free(pathShp);
	// 			dataLevel--;
	// 		}
	// 	}

	// 	pShapefileInfos = pShapefileInfos->next;
	// }
}

static void
darxen_renderer_render_overlay(DarxenRenderer *renderer)
{
	USING_PRIVATE(renderer);

	// SettingsSite* site = priv->radarSite;
	// DarxenRadarSiteInfo* siteInfo = darxen_radar_sites_get_site_info(site->name);

	// SettingsRenderer* renderSettings = &settings_get_main()->renderer;
	// float fltScale = renderSettings->fltScale;

	// GSList* pShapefileInfos = ((ProductInfo*)priv->products->data)->shapefiles;
	// while (pShapefileInfos)
	// {
	// 	ShapefileInfo* shapefileInfo = (ShapefileInfo*)pShapefileInfos->data;
	// 	DarxenShapefile* shapefile = shapefileInfo->shapefile;

	// 	if (shapefile->visible)
	// 	{
	// 		int dataLevel = 0;
	// 		if (shapefile->dataLevels)
	// 		{
	// 			GSList* pDataLevels = shapefile->dataLevels->levels;

	// 			while (pDataLevels)
	// 			{
	// 				DarxenShapefileDataLevel* level = (DarxenShapefileDataLevel*)pDataLevels->data;

	// 				if (level->scale > fltScale)
	// 					break;

	// 				dataLevel++;

	// 				pDataLevels = pDataLevels->next;
	// 			}
	// 			dataLevel--;
	// 			if (dataLevel < 0)
	// 			{
	// 				g_warning("Data level does not fit with any specified ranges, defaulting to 0");
	// 				dataLevel = 0;
	// 			}
	// 		}
	// 		while (dataLevel >= 0)
	// 		{
	// 			//FIXME: site location should be taken from the radar data itself?
	// 			//FIXME: shouldn't hardcode radius?
	// 			char* pathShp = darxen_shapefiles_filter_shp(shapefile, dataLevel, siteInfo->chrID, siteInfo->fltLatitude, siteInfo->fltLongitude, VIEW_AREA);

	// 			if (shapefile->polygons && shapefile->polygons->visible)
	// 			{
	// 				glColor4fv(shapefile->polygons->color);
	// 				if (!shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POLYGONS])
	// 				{
	// 					int list = glGenLists(1);

	// 					glNewList(list, GL_COMPILE_AND_EXECUTE);
	// 					{
	// 						darxen_shapefile_renderer_render_polygons(shapefile->polygons, pathShp, siteInfo, FALSE);
	// 					}
	// 					glEndList();

	// 					shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POLYGONS] = list;
	// 				}
	// 				else
	// 				{
	// 					glCallList(shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POLYGONS]);
	// 				}
	// 			}
	// 			if (shapefile->lines && shapefile->lines->visible)
	// 			{
	// 				glColor4fv(shapefile->lines->color);
	// 				if (!shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_LINES])
	// 				{
	// 					int list = glGenLists(1);

	// 					glNewList(list, GL_COMPILE_AND_EXECUTE);
	// 					{
	// 						darxen_shapefile_renderer_render_lines(shapefile->lines, pathShp, siteInfo, FALSE);
	// 					}
	// 					glEndList();

	// 					shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_LINES] = list;
	// 				}
	// 				else
	// 				{
	// 					glCallList(shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_LINES]);
	// 				}
	// 			}
	// 			if (shapefile->points && shapefile->points->visible)
	// 			{
	// 				glColor4fv(shapefile->points->color);
	// 				if (!shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POINTS])
	// 				{
	// 					int list = glGenLists(1);

	// 					glNewList(list, GL_COMPILE_AND_EXECUTE);
	// 					{
	// 						darxen_shapefile_renderer_render_points(shapefile->points, pathShp, siteInfo, FALSE);
	// 					}
	// 					glEndList();

	// 					shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POINTS] = list;
	// 				}
	// 				else
	// 				{
	// 					glCallList(shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_POINTS]);
	// 				}
	// 			}
	// 			if (shapefile->textLabels && shapefile->textLabels->visible)
	// 			{
	// 				glColor4fv(shapefile->textLabels->color);
	// 				if (!shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_TEXT_LABELS])
	// 				{
	// 					int list = glGenLists(1);

	// 					gchar* fontname = g_strdup_printf("courier new %.1f", shapefile->textLabels->fontSize);
	// 					DarxenGLFont* font = darxen_rendering_common_font_cache_get_font(priv->fonts, fontname);
	// 					g_free(fontname);

	// 					glNewList(list, GL_COMPILE_AND_EXECUTE);
	// 					{
	// 						darxen_shapefile_renderer_render_text_labels(shapefile->textLabels, pathShp, siteInfo, font, FALSE);
	// 					}
	// 					glEndList();

	// 					shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_TEXT_LABELS] = list;
	// 				}
	// 				else
	// 				{
	// 					glCallList(shapefileInfo->dspDataLevels[dataLevel][SHAPEFILE_PART_TEXT_LABELS]);
	// 				}
	// 			}

	// 			free(pathShp);
	// 			dataLevel--;
	// 		}
	// 	}

	// 	pShapefileInfos = pShapefileInfos->next;
	// }

	darxen_renderer_shared_render_overlay_legend(renderer);
}


static void
darxen_renderer_shared_render_overlay_legend(DarxenRenderer *renderer)
{
	USING_PRIVATE(renderer);

	// SettingsRenderer *renderSettings;
	// SettingsPalette *objPalette;
	// gint i;
	// int intBorderWidth = 2;
	// int intBoxSize = 20;
	// char *chrMessage = (char*)malloc(sizeof(char) * 25);

	// renderSettings = &settings_get_main()->renderer;
	// objPalette = renderSettings->objPaletteReflectivity;

	// glMatrixMode(GL_PROJECTION);
	// glPushMatrix();
	// glLoadIdentity();
	// glOrtho(0, priv->intWidth, priv->intHeight, 0, -1, 1);
	// glMatrixMode(GL_MODELVIEW);
	// glPushMatrix();
	// glLoadIdentity();

	// glColor3f(0.0f, 0.0f, 0.0f);
	// glRectf(0.0f, 0.0f, (float)(16 * intBoxSize + 2 * intBorderWidth), (float)(intBoxSize + 2 * intBorderWidth));
	// for (i = 0; i < 16; i++)
	// {
	// 	glColor4f(objPalette->fltRed[i], objPalette->fltGreen[i], objPalette->fltBlue[i], objPalette->fltAlpha[i]);
	// 	glRectf((float)(i * intBoxSize + intBorderWidth), (float)intBorderWidth, (float)((i + 1) * intBoxSize + intBorderWidth), (float)(intBoxSize + intBorderWidth));
	// }

	// sprintf(chrMessage, "VCP %i\nMX: %idBZ", priv->objData->objDescription.intVolCovPat, priv->objData->objDescription.intProdCodes[3]);
	// glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	// darxen_renderer_shared_render_text(chrMessage, TEXT_UPPER_LEFT, TRUE, 5.0f, (float)(intBoxSize + 2 * intBorderWidth + 5));
	// darxen_renderer_shared_render_text_line(darxen_conversions_format_date_time(priv->objData->objDescription.objScanDate, priv->objData->objDescription.intScanTime), TEXT_LOWER_RIGHT, TRUE, (float)(priv->intWidth - 3), (float)(priv->intHeight - 3));

	// glPopMatrix();
	// glMatrixMode(GL_PROJECTION);
	// glPopMatrix();
	// glMatrixMode(GL_MODELVIEW);
}

typedef enum
{
	MARCH_NONE,
	MARCH_UP,
	MARCH_DOWN,
	MARCH_LEFT,
	MARCH_RIGHT
} MarchDirection;

static gboolean
march(ProductsLevel3RadialDataPacket* objData, GLUtesselator* tess, GSList** sliceMem, int val, int az, int range, MarchDirection dir, gboolean* done[])
{
	if (az < 0)
		az += objData->intRadialCount;
	az %= objData->intRadialCount;
//	if (range < 0)
//		range += objData->intNumRangeBins;
//	range %= objData->intNumRangeBins;

	if (done[az][range] != -1)
	{
		if (done[az][range])
			return FALSE;
		done[az][range] = TRUE;
	}

	int aa = 0;
	int bb = 0;
	int cc = 0;
	int dd = 0;

	//determine the type of cell we are
	int cell = 0;
	int az1 = az ? (az - 1) : (objData->intRadialCount - 1);
	if (range >= 0)
	{
		aa = objData->objRadials[az].chrColorCode[range];
		bb = objData->objRadials[az1].chrColorCode[range];
		cell |= (aa >= val) << 0;
		cell |= (bb >= val) << 1;
	}
	if (range < objData->intNumRangeBins)
	{
		cc = objData->objRadials[az1].chrColorCode[range+1];
		dd = objData->objRadials[az].chrColorCode[range+1];
		cell |= (cc >= val) << 2;
		cell |= (dd >= val) << 3;
	}

	double d_az1 = -1.0;
	double d_range1 = -1.0;
	//double d_az2;
	//double d_range2;

	double threshold = 15;

	double a = aa;// / threshold;// + 0.01;
	double b = bb;// / threshold;// + 0.02;
	double c = cc;// / threshold;// + 0.03;
	double d = dd;// / threshold;// + 0.04;

	switch (cell)
	{
	case 0:
	case 15:
		return FALSE;
	//draw one
	case 1:
	case 14:
		d_az1 =		0.5;
		d_range1 =	1.0 + (d-a-2*val) / (2*threshold);
		break;
	case 2:
	case 13:
		d_az1 =		1.0 - (b-a-2*val) / (2*threshold);
		d_range1 =	0.5;
		break;
	case 4:
	case 11:
		d_az1 =		1.5;
		d_range1 =	1.0 - (c-b-2*val) / (2*threshold);
		break;
	case 7:
	case 8:
		d_az1 =		1.0 + (d-c-2*val) / (2*threshold);
		d_range1 =	1.5;
		break;
	//draw adjacent
	case 3:
		d_az1 =		0.5;
		d_range1 =	1.0 + (d-a-2*val) / (2*threshold);
		break;
	case 12:
		d_az1 =		0.5;
		d_range1 =	1.0 + (c-b-2*val) / (2*threshold);
		break;
	case 6:
		d_az1 =		1.0 + (a-b-2*val) / (2*threshold);
		d_range1 =	1.5;
		break;
	case 9:
		d_az1 =		1.0 + (d-c-2*val) / (2*threshold);
		d_range1 =	1.5;
		break;
	//draw opposite
	case 5:
		//split - move down/up
		done[az][range] = FALSE;
		switch (dir)
		{
		case MARCH_NONE:
			return FALSE;
		case MARCH_RIGHT:
			//go up
			d_az1 =		0.0;
			d_range1 =	0.5-0.5 * (threshold - ((a + d) / 2.0)) / (threshold - val);
			break;
		case MARCH_LEFT:
			//go down
			d_az1 =		0.5-0.5 * (threshold - ((a + b) / 2.0)) / (threshold - val);
			d_range1 =	0.0;
			break;
		default:
			g_error("Smoothing algorithm is broken");
		}
		break;
	case 10:
		//split - move left/right
		done[az][range] = FALSE;
		switch (dir)
		{
		case MARCH_NONE:
			return FALSE;
		case MARCH_UP:
			//go left
			d_az1 =		0.0;
			d_range1 =	0.5-0.5 * (threshold - ((a + d) / 2.0)) / (threshold - val);
			break;
		case MARCH_DOWN:
			//go right
			d_az1 =		0.5-0.5 * (threshold - ((c + d) / 2.0)) / (threshold - val);
			d_range1 =	1.0;
			break;
		default:
			g_error("Smoothing algorithm is broken");
		}
		break;
	default:
		break;
	}


	//determine which direction we will march next
	MarchDirection newDir = MARCH_NONE;
	switch (cell)
	{
	case 0:
	case 15:
		//either not touching the region, or completely inside it
		break;
	case 4:
	case 6:
	case 7:
		//move up
		newDir = MARCH_UP;
		break;
	case 8:
	case 12:
	case 14:
		//move left
		newDir = MARCH_LEFT;
		break;
	case 1:
	case 9:
	case 13:
		//move down
		newDir = MARCH_DOWN;
		break;
	case 2:
	case 3:
	case 11:
		//move right
		newDir = MARCH_RIGHT;
		break;
	case 5:
		//split - move down/up
		switch (dir)
		{
		case MARCH_RIGHT:
			//go up
			newDir = MARCH_UP;
			break;
		case MARCH_LEFT:
			//go down
			newDir = MARCH_DOWN;
			break;
		default:
			g_error("Smoothing algorithm is broken");
		}
		break;
	case 10:
		//split - move left/right
		switch (dir)
		{
		case MARCH_UP:
			//go left
			newDir = MARCH_LEFT;
			break;
		case MARCH_DOWN:
			//go right
			newDir = MARCH_RIGHT;
			break;
		default:
			g_error("Smoothing algorithm is broken");
		}
		break;
	}

	//switch point ordering if needed
	/*if ((dir == MARCH_LEFT && d_az1 < d_az2)
			|| (dir == MARCH_RIGHT && d_az2 < d_az1)
			|| (dir == MARCH_UP && d_range1 > d_range2)
			|| (dir == MARCH_DOWN && d_range2 > d_range1)
			|| ((dir == MARCH_NONE) &&
					((newDir == MARCH_LEFT && d_az1 < d_az2)
					|| (newDir == MARCH_RIGHT && d_az2 < d_az1)
					|| (newDir == MARCH_UP && d_range1 > d_range2)
					|| (newDir == MARCH_DOWN && d_range2 > d_range1))))
	{
		double d_tmp;
		d_tmp = d_az1;
		d_az1 = d_az2;
		d_az2 = d_tmp;

		d_tmp = d_range1;
		d_range1 = d_range2;
		d_range2 = d_tmp;
	}*/

	//render our point
	//d_range1 += 0.5;
	//d_range2 += 0.5;



	/*if (d_range1 > 500 || d_range1 < -0.5 ||
		d_az1 > 500 || d_az1 < -0.5)
		g_error("Fail");*/

	float kmPerRangeBin = 1.0f;
	ProductsLevel3Radial rad = objData->objRadials[az];
//	rad.fltAngleDelta = objData->objRadials[az1].fltAngleDelta;

	float theta1 = 90.0 - (rad.fltStartAngle + rad.fltAngleDelta) + d_az1 * rad.fltAngleDelta;// + 0.5;
//	float theta2 = 90.0 - (rad.fltStartAngle + rad.fltAngleDelta) + d_az2 * rad.fltAngleDelta + 0.5;
	double* pt = (double*)g_slice_alloc(3 * sizeof(double));
	pt[0] = (range + d_range1) * kmPerRangeBin * cos(DEG_TO_RAD(theta1));
	pt[1] = (range + d_range1) * kmPerRangeBin * sin(DEG_TO_RAD(theta1));
	pt[2] = 0.0;
	*sliceMem = g_slist_prepend(*sliceMem, pt);
	gluTessVertex(tess, pt, pt);
	glVertex2f((range + d_range1) * kmPerRangeBin * cos(DEG_TO_RAD(theta1)), (range + d_range1) * kmPerRangeBin * sin(DEG_TO_RAD(theta1)));
//	glVertex2f((range + d_range2) * kmPerRangeBin * cos(DEG_TO_RAD(theta2)), (range + d_range2) * kmPerRangeBin * sin(DEG_TO_RAD(theta2)));


	//march onward
	switch (newDir)
	{
	case MARCH_LEFT:
		march(objData, tess, sliceMem, val, az+1, range, MARCH_LEFT, done);
		break;
	case MARCH_RIGHT:
		march(objData, tess, sliceMem, val, az-1, range, MARCH_RIGHT, done);
		break;
	case MARCH_UP:
		march(objData, tess, sliceMem, val, az, range+1, MARCH_UP, done);
		break;
	case MARCH_DOWN:
		march(objData, tess, sliceMem, val, az, range-1, MARCH_DOWN, done);
		break;
	default:
		g_error("Invalid newDir");
	}

	return TRUE;

}



static void APIENTRY
tessBegin(GLenum type)
{
	glBegin(type);
}

static void APIENTRY
tessVertex(GLvoid *vertex)
{
	const GLdouble *pointer;
	pointer = (GLdouble*)vertex;
	glVertex3dv(pointer);
}

static void APIENTRY
tessEnd()
{
	glEnd();
}

static void APIENTRY
tessError(GLenum errno)
{
	g_error("Tess Error: %i\n", errno);
}

static void APIENTRY
tessCombine(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], GLdouble **dataOut, void* user_data)
{

	//g_error("Tess Combine not implemented");
   GLdouble *vertex;
//   int i;

   vertex = (GLdouble*)g_slice_alloc(3 * sizeof(GLdouble));
   vertex[0] = coords[0];
   vertex[1] = coords[1];
   vertex[2] = coords[2];
   /*vertex[3] = 0.0f; //r
   vertex[4] = 0.0f; //g
   vertex[5] = 0.0f; //b
   vertex[6] = 0.0f; //a
   for (i = 0; i < 4; i++)
   {
	   if (vertex_data[i])
	   {
		   vertex[3] += weight[0] * vertex_data[i][3];
		   vertex[4] += weight[1] * vertex_data[i][4];
		   vertex[5] += weight[2] * vertex_data[i][5];
		   vertex[6] += weight[3] * vertex_data[i][6];
	   }
   }*/

   *dataOut = vertex;

   *(GSList**)user_data = g_slist_prepend(*(GSList**)user_data, vertex);
}


static void
darxen_renderer_render_radial_data(DarxenRenderer *renderer, ProductsLevel3RadialDataPacket *objData)
{
	const DarxenPalette* palette;
	int i;
	int az;
	int range;
	float fltStart;
	float fltEnd;
	float CosX1;
	float SinY1;
	float CosX2;
	float SinY2;

	float kmPerRangeBin = 1.0f;//0.25f;

	USING_PRIVATE(renderer);

	palette = darxen_palette_get_from_file("palettes/reflectivity.palette");
	g_assert(palette);

	glPushMatrix();
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glRotatef(90.0f - objData->objRadials[0].fltStartAngle, 0.0f, 0.0f, 1.0f);
		glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
		glBegin(GL_LINES);
		{
			glVertex2f(-0.5f, 0.0f);
			glVertex2f(0.5f, 0.0f);
			glVertex2f(0.0f, -0.5f);
			glVertex2f(0.0f, 1.9f);
		}
		glEnd();
		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_TRIANGLES);
		{
			glVertex2f(-0.5f, 0.0f);
			glVertex2f(0.0f, 2.0f);
			glVertex2f(0.5f, 0.0f);
		}
		glEnd();
	}
	glPopMatrix();

	if (priv->product.smoothing)
	{
		//TODO: generate polygons using marching squares
		GLUtesselator* tess = gluNewTess();
		gluTessCallback(tess, GLU_TESS_BEGIN, tessBegin);
		gluTessCallback(tess, GLU_TESS_VERTEX, tessVertex);
		gluTessCallback(tess, GLU_TESS_END, tessEnd);
		gluTessCallback(tess, GLU_TESS_ERROR, tessError);
		gluTessCallback(tess, GLU_TESS_COMBINE_DATA, tessCombine);

		for (i = 1; i < 16; i++)
		{
			gboolean* done_mask[objData->intRadialCount];
			gboolean done_radials[objData->intRadialCount * objData->intNumRangeBins];
			for (az = 0; az < objData->intRadialCount; az++)
			{
				done_mask[az] = &done_radials[az * objData->intNumRangeBins];
				for (range = 0; range < objData->intNumRangeBins; range++)
					done_mask[az][range] = FALSE;
			}

			glColor4fv((float*)(palette->colors+i));

			GSList* lstCombined = NULL;

			gluTessBeginPolygon(tess, &lstCombined);
			{
				for (az = 0; az < objData->intRadialCount; az++)
				{
					for (range = 0; range < objData->intNumRangeBins - 1; range++)
					{
						gluTessBeginContour(tess);
						glBegin(GL_LINE_LOOP);
						{
								march(objData, tess, &lstCombined, i, az, range, MARCH_NONE, done_mask);
						}
						glEnd();
						gluTessEndContour(tess);
					}
				}
			}
			gluTessEndPolygon(tess);

			GSList* plstCombined = lstCombined;
			while (plstCombined)
			{
				g_slice_free1(3*sizeof(double), plstCombined->data);
				plstCombined = plstCombined->next;
			}
			g_slist_free(lstCombined);
		}

		gluDeleteTess(tess);


	}
	else
	{
		int intRangeStart;
		int intRangeStartColor;

		glBegin(GL_QUADS);
		{
			for (i = 0; i < 16; i++)
			{
				/* set the current color */
				glColor4fv((float*)(palette->colors+i));
				for (az = 0; az < objData->intRadialCount; az++)
				{
					/* convert to our coordinate system.
					 * The data file uses 0 degrees as north, with positive angles being clockwise.
					 * We use the standard unit circle
					 */
					fltStart = 90.0 - (objData->objRadials[az].fltStartAngle + objData->objRadials[az].fltAngleDelta);
					fltEnd = fltStart + objData->objRadials[az].fltAngleDelta;
					CosX1 = (float)cos(DEG_TO_RAD(fltStart));
					SinY1 = (float)sin(DEG_TO_RAD(fltStart));
					CosX2 = (float)cos(DEG_TO_RAD(fltEnd));
					SinY2 = (float)sin(DEG_TO_RAD(fltEnd));

					intRangeStart = 0;
					intRangeStartColor = 0;
					for (range = 0; range < objData->intNumRangeBins; range++)
					{
						if (objData->objRadials[az].chrColorCode[range] == i)
						{
							glVertex2f(range * kmPerRangeBin * CosX1, range * kmPerRangeBin * SinY1);
							glVertex2f((range + 1.0f) * kmPerRangeBin * CosX1, (range + 1.0f) * kmPerRangeBin * SinY1);
							glVertex2f((range + 1.0f) * kmPerRangeBin * CosX2, (range + 1.0f) * kmPerRangeBin * SinY2);
							glVertex2f(range * kmPerRangeBin * CosX2, range * kmPerRangeBin * SinY2);
						}
					}
				}
			}
		}
		glEnd();
	}
}


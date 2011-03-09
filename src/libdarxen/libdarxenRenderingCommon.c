/*
 * libdarxenRenderingCommon.c
 *
 *  Created on: Mar 17, 2010
 *      Author: wellska1
 */

#define G_LOG_DOMAIN "libdarxenRenderingCommon"
#include "glib/gmessages.h"

#include "libdarxenRenderingCommon.h"

#include <glib/ghash.h>
#include <pango/pango.h>
#include <gdk/gdkglfont.h>

#include <string.h>
#include <malloc.h>

struct _DarxenGLFontCache
{
	GHashTable* fonts; //DarxenGLFont
};

static float* latLonPoint = NULL;

static void	project_geocentric_rectangular_to_local_enu		(double geodeticCenterLat, double geodeticCenterLon, double* geocentricCenter, double* offset, double* res);
static void	project_lat_lon_to_geocentric_rectangular		(double lat, double lon, double* xy);

DarxenGLFontCache*
darxen_rendering_common_font_cache_new()
{
	DarxenGLFontCache* cache = (DarxenGLFontCache*)malloc(sizeof(DarxenGLFontCache));
	cache->fonts = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	return cache;
}

void
darxen_rendering_common_font_cache_free(DarxenGLFontCache* cache)
{
	GHashTableIter iter;
	DarxenGLFont* font;
	g_hash_table_iter_init(&iter, cache->fonts);
	while (g_hash_table_iter_next(&iter, NULL, (gpointer*)&font))
	{
		glDeleteLists(font->dspTextBase, 128);
		g_hash_table_iter_remove(&iter);
	}
	g_hash_table_destroy(cache->fonts);
	free(cache);
}

DarxenGLFont*
darxen_rendering_common_font_cache_get_font(DarxenGLFontCache* cache, const char* desc)
{
	DarxenGLFont* font = (DarxenGLFont*)g_hash_table_lookup(cache->fonts, desc);

	if (!font)
	{
		PangoFont *pango_font;
		PangoFontDescription *font_desc;

		font = (DarxenGLFont*)malloc(sizeof(DarxenGLFont));

		font->dspTextBase = glGenLists(128);
		font_desc = pango_font_description_from_string(desc);
		pango_font = gdk_gl_font_use_pango_font(font_desc, 0, 128, font->dspTextBase);
		pango_font_description_free(font_desc);
		if (!pango_font)
		{
			g_critical("Could not create font display lists");
			return NULL;
		}
		PangoFontMetrics* metrics = pango_font_get_metrics(pango_font, NULL);
		if (!pango_font)
		{
			g_critical("Could not retrieve font metrics");
			return NULL;
		}
		font->width = pango_font_metrics_get_approximate_char_width(metrics) / PANGO_SCALE;
		font->ascent = pango_font_metrics_get_ascent(metrics) / PANGO_SCALE;
		font->descent = pango_font_metrics_get_descent(metrics) / PANGO_SCALE;
		font->height = font->ascent ;
		pango_font_metrics_unref(metrics);

		g_hash_table_insert(cache->fonts, strdup(desc), font);
	}

	return font;
}

void
darxen_rendering_common_font_cache_free_font(DarxenGLFontCache* cache, const char* desc)
{
	DarxenGLFont* font = (DarxenGLFont*)g_hash_table_lookup(cache->fonts, desc);
	g_assert(font);

	glDeleteLists(font->dspTextBase, 128);
	g_hash_table_remove(cache->fonts, desc);
}

void
darxen_rendering_common_draw_string(const char* message, DarxenTextOrigin origin, float x, float y, DarxenGLFont* font, GLfloat* color, gboolean drawBackground)
{
	if (!font || !font->dspTextBase)
	{
		g_warning("Invalid font");
		return;
	}

	int len = strlen(message);
	float textWidth;

	if (drawBackground)
	{
		if (color)
			glColor4f(1.0f - color[0], 1.0f - color[1], 1.0f - color[2], color[3]);

		textWidth = (font->width - 2) * len + 4;
		switch (origin)
		{
		case TEXT_ORIGIN_UPPER_LEFT:
			glRectf(x, y, x + textWidth, y + font->height + 4);
			break;
		case TEXT_ORIGIN_LOWER_RIGHT:
			glRectf(x - textWidth, y - (font->height + 4), x, y);
			break;
		case TEXT_ORIGIN_UPPER_RIGHT:
			glRectf(x - textWidth, y, x, y + font->height + 4);
			break;
		case TEXT_ORIGIN_LOWER_LEFT:
			glRectf(x, y - (font->height + 4), x + textWidth, y);
			break;
		}

		if (!color)
			glPopAttrib();
	}

	if (color)
		glColor4fv(color);

	glRasterPos2f(x, y);

	int offsetX = 0;
	int offsetY = 0;
	textWidth = (font->width - 2) * len + 2;

	switch (origin)
	{
	case TEXT_ORIGIN_UPPER_LEFT:
		offsetX = 2;
		offsetY = 2 + font->height;
		glRasterPos2f(x + 2, y + 2 + font->height);
		break;
	case TEXT_ORIGIN_LOWER_RIGHT:
		offsetX = -(font->width * len + 2);
		offsetY = -2;
		glRasterPos2f(x - textWidth, y - 2);
		break;
	case TEXT_ORIGIN_UPPER_RIGHT:
		offsetX = -(font->width * len + 2);
		offsetY = 2 + font->height;
		glRasterPos2f(x - textWidth, y + 2 + font->height);
		break;
	case TEXT_ORIGIN_LOWER_LEFT:
		offsetX = 2;
		offsetY = -2;
		glRasterPos2f(x + 2, y - 2);
		break;
	}
	//glBitmap(0.0f, 0.0f, 0.0f, 0.0f, offsetX, offsetY, NULL);
	glListBase(font->dspTextBase);
	glCallLists(len, GL_UNSIGNED_BYTE, message);
}

float*
darxen_rendering_common_convert_lon_lat_to_x_y(float latCenter, float lonCenter, float lat, float lon)
{
	if (!latLonPoint)
		latLonPoint = (float*)calloc(2, sizeof(float));

	double center[3];
	double offset[3];

	project_lat_lon_to_geocentric_rectangular(latCenter, lonCenter, center);
	project_lat_lon_to_geocentric_rectangular(lat, lon, offset);

	double res[3];
	project_geocentric_rectangular_to_local_enu(latCenter, lonCenter, center, offset, res);

	latLonPoint[0] = res[0];
	latLonPoint[1] = res[1];

	return latLonPoint;
}

/*********************
 * Private Functions *
 *********************/


void
project_geocentric_rectangular_to_local_enu(double geodeticCenterLat, double geodeticCenterLon,
											double* geocentricCenter, double* offset, double* res)
{
	//latitude - theta
	//longitude - lambda

	double offsetX = offset[0] - geocentricCenter[0];
	double offsetY = offset[1] - geocentricCenter[1];
	double offsetZ = offset[2] - geocentricCenter[2];

	double resX = 	offsetX * -degsin(geodeticCenterLon) +
					offsetY * degcos(geodeticCenterLon);
	double resY = 	offsetX * -degsin(geodeticCenterLat) * degcos(geodeticCenterLon) +
					offsetY * -degsin(geodeticCenterLat) * degsin(geodeticCenterLon) +
					offsetZ * degcos(geodeticCenterLat);
	double resZ = 	offsetX * degcos(geodeticCenterLat) * degcos(geodeticCenterLon) +
					offsetY * degcos(geodeticCenterLat) * degsin(geodeticCenterLon) +
					offsetZ * degsin(geodeticCenterLat);
	res[0] = resX;
	res[1] = resY;
	res[2] = resZ;
}

void
project_lat_lon_to_geocentric_rectangular(double lat, double lon, double* xyz)
{
	//Using equation from http://en.wikipedia.org/wiki/Reference_ellipsoid
	//Using datum (GRS80) from http://en.wikipedia.org/wiki/NAD83

	static double a = 6378137.0;
	static double b = 6356752.314140;

	double ae = degacos(b / a);

	double height = 0.0;

	double inner = degsin(lat) * degsin(ae);

	double N = a / sqrt(1.0 - inner * inner);

	double x = (N + height) * degcos(lat) * degcos(lon);
	double y = (N + height) * degcos(lat) * degsin(lon);
	double z = (pow(degcos(ae), 2.0) * N + height) * degsin(lat);

	//to km
	x /= 1000.0;
	y /= 1000.0;
	z /= 1000.0;

	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;
}


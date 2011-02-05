/*
 * libdarxenRenderingCommon.h
 *
 *  Created on: Mar 17, 2010
 *      Author: wellska1
 */

#ifndef LIBDARXENRENDERINGCOMMON_H_
#define LIBDARXENRENDERINGCOMMON_H_

#include "libdarxenCommon.h"

#include <GL/glu.h>
#include <GL/gl.h>

#include <math.h>

#define DARXEN_FONT_BASE	"courier new 8"

typedef struct _DarxenGLFont		DarxenGLFont;
typedef struct _DarxenGLFontCache	DarxenGLFontCache;

typedef enum
{
	TEXT_ORIGIN_UPPER_LEFT,
	TEXT_ORIGIN_UPPER_RIGHT,
	TEXT_ORIGIN_LOWER_LEFT,
	TEXT_ORIGIN_LOWER_RIGHT
} DarxenTextOrigin;

struct _DarxenGLFont
{
	GLuint dspTextBase;
	int width;
	int height;
	int ascent;
	int descent;
};

DarxenGLFontCache*	darxen_rendering_common_font_cache_new			();
void				darxen_rendering_common_font_cache_free			(DarxenGLFontCache* cache);
DarxenGLFont*		darxen_rendering_common_font_cache_get_font		(DarxenGLFontCache* cache, const char* desc);
void				darxen_rendering_common_font_cache_free_font	(DarxenGLFontCache* cache, const char* desc);

void				darxen_rendering_common_draw_string				(const char* message, DarxenTextOrigin origin, float x, float y, DarxenGLFont* font, GLfloat* color, gboolean drawBackground);

float*				darxen_rendering_common_convert_lon_lat_to_x_y	(float latCenter, float lonCenter, float lat, float lon);


#endif /* LIBDARXENRENDERINGCOMMON_H_ */

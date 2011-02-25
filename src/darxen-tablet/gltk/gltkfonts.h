/* gltkfonts.h
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

#ifndef GLTKFONTS_H_GA3WY73V
#define GLTKFONTS_H_GA3WY73V

#include <FTGL/ftgl.h>

#include <glib.h>
#include <GL/gl.h>

typedef struct _GltkGLFont		GltkGLFont;
typedef struct _GltkGLFontCache	GltkGLFontCache;

struct _GltkGLFont
{
	FTGLfont* font;
	float ascender;
};

typedef enum
{
	TEXT_ORIGIN_UPPER_LEFT,
	TEXT_ORIGIN_UPPER_RIGHT,
	TEXT_ORIGIN_LOWER_LEFT,
	TEXT_ORIGIN_LOWER_RIGHT
} GltkGLFontTextOrigin;

#define 			GLTK_FONTS_BASE					"UnBatang.ttf"

GltkGLFontCache*	gltk_fonts_cache_new			();
void				gltk_fonts_cache_free			(GltkGLFontCache* cache);
GltkGLFont*			gltk_fonts_cache_get_font		(GltkGLFontCache* cache, const char* path, int size);

//void				gltk_fonts_test_string			(const char* message);
#endif


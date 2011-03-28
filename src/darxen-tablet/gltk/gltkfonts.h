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

typedef struct _GltkGLFontBounds	GltkGLFontBounds;
typedef struct _GltkGLFont			GltkGLFont;

struct _GltkGLFontBounds
{
	float width;
	float height;
};

struct _GltkGLFont
{
	FT_Face	face;
	GltkGLFontBounds* glyphs;
	FTGLfont* font;
	float ascender;
	float descender;
};

#define 			GLTK_FONTS_BASE					"UnBatang.ttf"

void				gltk_fonts_cache_free			();
GltkGLFont*			gltk_fonts_cache_get_font		(const char* path, int size, gboolean renderable);
GltkGLFontBounds	gltk_fonts_measure_string		(GltkGLFont* font, const char* txt);


//void				gltk_fonts_test_string			(const char* message);
#endif


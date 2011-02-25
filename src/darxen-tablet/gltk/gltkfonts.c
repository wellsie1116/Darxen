/* gltkfonts.c
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

#include "gltkfonts.h"

struct _GltkGLFontCache
{
	GHashTable* fonts;
};

void	free_font(GltkGLFont* font);

GltkGLFontCache*
gltk_fonts_cache_new()
{
	GltkGLFontCache* cache = g_new(GltkGLFontCache, 1);
	cache->fonts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free_font);
	return cache;
}

void
gltk_fonts_cache_free(GltkGLFontCache* cache)
{
	g_hash_table_destroy(cache->fonts);
	free(cache);
}

GltkGLFont*
gltk_fonts_cache_get_font(GltkGLFontCache* cache, const char* path, int size)
{
	gchar* desc = g_strdup_printf("%s %i", path, size);

	GltkGLFont* font = (GltkGLFont*)g_hash_table_lookup(cache->fonts, desc);

	if (!font)
	{
		FTGLfont* ftglFont = ftglCreateTextureFont(path);
		if (!ftglFont)
			return NULL;

		font = g_new(GltkGLFont, 1);

		font->font = ftglFont;
		ftglSetFontFaceSize(font, size, size);
		font->ascender = ftglGetFontAscender(font);

		g_hash_table_insert(cache->fonts, desc, font);
	}

	return font;
}


//void
//gltk_fonts_test_string			(const char* message)
//{
//	FTGLfont* font = ftglCreateTextureFont("UnBatang.ttf");
//	g_assert(font);
//
//	ftglSetFontFaceSize(font, 24, 24);
//	float height = ftglGetFontAscender(font);
//
//	glPushMatrix();
//	{
//		glTranslatef(0.0f, -height, 0.0f);
//		ftglRenderFont(font, message, FTGL_RENDER_ALL);
//	}
//	glPopMatrix();
//
//
//	ftglDestroyFont(font);
//}

void
free_font(GltkGLFont* font)
{
	ftglDestroyFont(font->font);
}


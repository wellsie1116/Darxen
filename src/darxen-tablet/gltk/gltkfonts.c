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

static GHashTable* fonts = NULL;

void	free_font(GltkGLFont* font);

void
gltk_fonts_cache_free()
{
	g_hash_table_destroy(fonts);
}

GltkGLFont*
gltk_fonts_cache_get_font(const char* path, int size, gboolean renderable)
{
	gchar* desc = g_strdup_printf("%s %i", path, size);
	if (!fonts)
		fonts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)free_font);

	GltkGLFont* font = (GltkGLFont*)g_hash_table_lookup(fonts, desc);

	FTGLfont* ftglFont;
	if (!font || (!font->rendered && renderable))
	{
		ftglFont = ftglCreateTextureFont(path);
		g_assert(ftglFont);
		ftglSetFontFaceSize(ftglFont, size, size);
	}

	if (!font)
	{
		FTGLfont* ftglFont = ftglCreateTextureFont(path);
		g_assert(ftglFont);

		font = g_new(GltkGLFont, 1);

		font->font = ftglFont;
		font->ascender = ftglGetFontAscender(font->font);
		font->descender = ftglGetFontDescender(font->font);
		font->rendered = renderable;

		g_hash_table_insert(fonts, desc, font);
	}
	else if (!font->rendered && renderable)
	{
		ftglDestroyFont(font->font);
		font->font = ftglFont;
		font->ascender = ftglGetFontAscender(font->font);
		font->descender = ftglGetFontDescender(font->font);
		font->rendered = TRUE;
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


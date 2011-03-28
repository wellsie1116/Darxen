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

static FT_Library library;

void	free_font(GltkGLFont* font);

void
gltk_fonts_cache_free()
{
	g_hash_table_destroy(fonts);
}

static FTGLfont*
create_ftgl_font(const char* path, int size)
{
	FTGLfont* ftglFont = ftglCreateTextureFont(path);
	g_assert(ftglFont);
	ftglSetFontFaceSize(ftglFont, size, size);

	return ftglFont;
}

GltkGLFont*
gltk_fonts_cache_get_font(const char* path, int size, gboolean renderable)
{
	if (!fonts)
	{
		fonts = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)free_font);
		g_assert(!FT_Init_FreeType(&library));
	}

	gchar* desc = g_strdup_printf("%s %i", path, size);
	GltkGLFont* font = (GltkGLFont*)g_hash_table_lookup(fonts, desc);

	if (!font)
	{
		font = g_slice_new(GltkGLFont);

		font->font = renderable ? create_ftgl_font(path, size) : NULL;
		font->ascender = renderable ? ftglGetFontAscender(font->font) : 0;
		font->descender = renderable ? ftglGetFontDescender(font->font) : 0;

		int error = FT_New_Face(library, path, 0, &font->face);
		g_assert(!error);
		error = FT_Set_Char_Size(font->face, 0, size*64, 0, 0);
		g_assert(!error);

		{
			font->glyphs = g_new(GltkGLFontBounds, 128);
			int i;
			for (i = 0; i < 128; i++)
			{
				FT_Glyph glyph;
				FT_BBox bbox;

				error = FT_Load_Glyph(font->face, FT_Get_Char_Index(font->face, i), FT_LOAD_DEFAULT);
				g_assert(!error);

				error = FT_Get_Glyph(font->face->glyph, &glyph);
				g_assert(!error);

				FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &bbox);

				font->glyphs[i].width = bbox.xMax - bbox.xMin + 1;
				font->glyphs[i].height = bbox.yMax - bbox.yMin + 1;

				//FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
				//FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
				//
				//font->glyphs[i].width = bitmap_glyph->bitmap.width;
				//font->glyphs[i].height = bitmap_glyph->bitmap.rows;
				//TODO memory management with freetype objects
			}
		}

		g_hash_table_insert(fonts, g_strdup(desc), font);
	}
	else if (!font->font && renderable)
	{
		font->font = create_ftgl_font(path, size);
		font->ascender = ftglGetFontAscender(font->font);
		font->descender = ftglGetFontDescender(font->font);
	}

	g_free(desc);
	return font;
}

GltkGLFontBounds
gltk_fonts_measure_string(GltkGLFont* font, const char* txt)
{
	GltkGLFontBounds res = {0,0};
	
	int lineWidth = 0;
	int lineHeight = 0;
	const char* pTxt;
	for (pTxt = txt; *pTxt; pTxt++)
	{
		if (*pTxt == '\n')
		{
			res.width = MAX(res.width, lineWidth);
			res.height = MAX(res.height, lineHeight);
			lineWidth = 0;
			lineHeight = 0;
		}
		else
		{
			lineWidth += font->glyphs[(int)*pTxt].width;
			lineHeight = MAX(lineHeight, font->glyphs[(int)*pTxt].height);
		}
	}
	res.width = MAX(res.width, lineWidth);
	res.height = MAX(res.height, lineHeight);

	return res;
}

void
free_font(GltkGLFont* font)
{
	FT_Done_Face(font->face);
	if (font->font)
		ftglDestroyFont(font->font);
	g_free(font->glyphs);
	g_slice_free(GltkGLFont, font);
}


/* DarxenRendererShared.c

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

#define G_LOG_DOMAIN "DarxenRendererShared"
#include <glib/gmessages.h>

#include "DarxenRendererShared.h"

#include <stdlib.h>
#include <math.h>

#include <gtk/gtkglwidget.h>

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#ifndef CALLBACK
#define CALLBACK
#endif

/* Private Variables */
static guint intTextBase = 0;

/********************
 * Public Functions *
 ********************/

void
darxen_renderer_shared_init_text()
{
	PangoFont *font;
	PangoFontDescription *font_desc;

	intTextBase = glGenLists(128);
	font_desc = pango_font_description_from_string("courier new 12");
	font = gdk_gl_font_use_pango_font(font_desc, 0, 128, intTextBase);
	if (!font)
		g_critical("Could not create font display lists");
	pango_font_description_free(font_desc);
}

void
darxen_renderer_shared_render_text(const char *chrMessage, DarxenTextLocation location, gboolean blnBackground, float x, float y)
{
	gint intStrLen;
	gint intFontHeight = 10;
//	gint intFontWidth = 10;
	char *chrLine;

	intStrLen = (gint)strlen(chrMessage);
	chrLine = strtok((char*)chrMessage, "\n");

	while (chrLine)
	{
		darxen_renderer_shared_render_text_line(chrLine, location, blnBackground, x, y);
		y += intFontHeight + 4;
		chrLine = strtok(NULL, "\n");
	}
}

void
darxen_renderer_shared_render_text_line(const char *chrLine, DarxenTextLocation location, gboolean blnBackground, float x, float y)
{
	gint intFontHeight = 10;
	gint intFontWidth = 10;
	int intStrLen = (int)strlen(chrLine);

	if (blnBackground)
	{
		GLfloat fltColor[4];
		glGetFloatv(GL_CURRENT_COLOR, fltColor);
		glPushAttrib(GL_CURRENT_BIT);
			glColor4f(1.0f - fltColor[0], 1.0f - fltColor[1], 1.0f - fltColor[2], fltColor[3]);
			if (location == TEXT_UPPER_LEFT)
				glRectf(x, y, x + intFontWidth * intStrLen + 4, y + intFontHeight + 4);
			else if (location == TEXT_LOWER_RIGHT)
				glRectf(x - (intFontWidth * intStrLen + 4), y - (intFontHeight + 4), x, y);
			else if (location == TEXT_UPPER_RIGHT)
				glRectf(x - (intFontWidth * intStrLen + 4), y, x, y + intFontHeight + 4);
			else if (location == TEXT_LOWER_LEFT)
				glRectf(x, y - (intFontHeight + 4), x + intFontHeight * intStrLen + 4, y);
		glPopAttrib();
	}

	glPushAttrib(GL_LIST_BIT);
		if (location == TEXT_UPPER_LEFT)
			glRasterPos2f(x + 2, y + 2 + intFontHeight);
		else if (location == TEXT_LOWER_RIGHT)
			glRasterPos2f(x - (intFontWidth * intStrLen + 2), y - 2);
		else if (location == TEXT_UPPER_RIGHT)
			glRasterPos2f(x - (intFontWidth * intStrLen + 2), y + 2 + intFontHeight);
		else if (location == TEXT_LOWER_LEFT)
			glRasterPos2f(x + 2, y - 2);
		if (!intTextBase)
		{
			g_critical("Text not initialized");
		}
		else
		{
			glListBase(intTextBase);
			glCallLists(intStrLen, GL_UNSIGNED_BYTE, chrLine);
		}
	glPopAttrib();
}


/********************
* Private Functions *
*********************/


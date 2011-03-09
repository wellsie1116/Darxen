/* gltklabel.c
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

#include "gltklabel.h"

#include "gltkfonts.h"
#include <GL/gl.h>

G_DEFINE_TYPE(GltkLabel, gltk_label, GLTK_TYPE_WIDGET)

#define USING_PRIVATE(obj) GltkLabelPrivate* priv = GLTK_LABEL_GET_PRIVATE(obj)
#define GLTK_LABEL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_LABEL, GltkLabelPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkLabelPrivate		GltkLabelPrivate;
struct _GltkLabelPrivate
{
	gchar* text;

	gboolean drawBorder;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void gltk_label_dispose(GObject* gobject);
static void gltk_label_finalize(GObject* gobject);

static void gltk_label_size_request(GltkWidget* widget, GltkSize* size);
static void gltk_label_render(GltkWidget* label);

static void
gltk_label_class_init(GltkLabelClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkLabelPrivate));
	
	gobject_class->dispose = gltk_label_dispose;
	gobject_class->finalize = gltk_label_finalize;

	gltkwidget_class->size_request = gltk_label_size_request;
	gltkwidget_class->render = gltk_label_render;
}

static void
gltk_label_init(GltkLabel* self)
{
	USING_PRIVATE(self);

	priv->text = NULL;
	priv->drawBorder = FALSE;
}

static void
gltk_label_dispose(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_label_parent_class)->dispose(gobject);
}

static void
gltk_label_finalize(GObject* gobject)
{
	GltkLabel* self = GLTK_LABEL(gobject);
	USING_PRIVATE(self);

	if (priv->text)
		g_free(priv->text);

	G_OBJECT_CLASS(gltk_label_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_label_new(const gchar* text)
{
	GObject *gobject = g_object_new(GLTK_TYPE_LABEL, NULL);
	GltkLabel* self = GLTK_LABEL(gobject);

	USING_PRIVATE(self);

	if (text)
		priv->text = g_strdup(text);

	return (GltkWidget*)gobject;
}

void
gltk_label_set_text	(GltkLabel* label, const gchar* text)
{
	g_return_if_fail(GLTK_IS_LABEL(label));
	USING_PRIVATE(label);
	if (priv->text)
		g_free(priv->text);
	priv->text = g_strdup(text);
	//TODO: render
}

void
gltk_label_set_draw_border(GltkLabel* label, gboolean drawBorder)
{
	g_return_if_fail(GLTK_IS_LABEL(label));
	USING_PRIVATE(label);
	priv->drawBorder = drawBorder;
	//TODO: render
}

GQuark
gltk_label_error_quark()
{
	return g_quark_from_static_string("gltk-label-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_label_size_request(GltkWidget* widget, GltkSize* size)
{
	USING_PRIVATE(widget);
	GltkGLFont* font = gltk_fonts_cache_get_font(GLTK_FONTS_BASE, 16, FALSE);
	g_assert(font);

	size->height = 20;
	size->width = 0;

	gchar** lines = g_strsplit(priv->text, "\n", -1);
	gchar** pLines = lines;
	while (*pLines)
	{
		GltkGLFontBounds bounds = gltk_fonts_measure_string(font, *pLines);
		// float bbox[6];
		// ftglGetFontBBox(font->font, *pLines, -1, bbox);
		// float width = bbox[3] - bbox[0];
		// float height = bbox[4] - bbox[1];

		//printf("Request line size: %f %f\n", bounds.width, bounds.height);

		size->height += bounds.height + 5;
		size->width = MAX(size->width, bounds.width);

		pLines++;
	}
	g_strfreev(lines);

	size->width += 20;

	// g_message("Request: %i %i", size->width, size->height);

	GLTK_WIDGET_CLASS(gltk_label_parent_class)->size_request(widget, size);
}

static void
gltk_label_render(GltkWidget* label)
{
	USING_PRIVATE(label);

	GltkAllocation allocation = gltk_widget_get_allocation(GLTK_WIDGET(label));
	// g_message("Allocation: %i %i %i %i", allocation.x, allocation.y, allocation.width, allocation.height);

	if (priv->drawBorder)
	{
		glBegin(GL_LINE_LOOP);
		{
			glColor3f(1.0f, 1.0f, 1.0f);

			glVertex2i(0, 0);
			glVertex2i(0, allocation.height);
			glVertex2i(allocation.width, allocation.height);
			glVertex2i(allocation.width, 0);
		}
		glEnd();
	}

	float width = allocation.width - 20;
	if (width < 5.0f)
		return;

	glPushMatrix();
	{
		GltkGLFont* font = gltk_fonts_cache_get_font(GLTK_FONTS_BASE, 16, TRUE);
		glColor3f(1.0f, 1.0f, 1.0f);
		glTranslatef(10.0f, font->ascender + font->descender + 10, 0.1f);
		glScalef(1.0f, -1.0f, 1.0f);

		gchar** lines = g_strsplit(priv->text, "\n", -1);
		gchar** pLines = lines;
		while (*pLines)
		{
			ftglRenderFont(font->font, *pLines, FTGL_RENDER_ALL);
			glTranslatef(0.0f, -(font->ascender + font->descender + 5), 0.0f);
			pLines++;
		}
		g_strfreev(lines);
	}
	glPopMatrix();
}


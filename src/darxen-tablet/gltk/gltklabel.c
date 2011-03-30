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
	gboolean drawBorder;
	int fontSize;
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

	self->text = NULL;
	self->color.r = 1.0f;
	self->color.g = 1.0f;
	self->color.b = 1.0f;

	priv->drawBorder = FALSE;
	priv->fontSize = 16;
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

	if (self->text)
		g_free(self->text);

	G_OBJECT_CLASS(gltk_label_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_label_new(const gchar* text)
{
	GObject *gobject = g_object_new(GLTK_TYPE_LABEL, NULL);
	GltkLabel* self = GLTK_LABEL(gobject);

	if (text)
		self->text = g_strdup(text);

	return (GltkWidget*)gobject;
}

void
gltk_label_set_text	(GltkLabel* label, const gchar* text)
{
	g_return_if_fail(GLTK_IS_LABEL(label));
	if (label->text)
		g_free(label->text);
	label->text = g_strdup(text);
	gltk_widget_invalidate(GLTK_WIDGET(label));
}

void
gltk_label_set_draw_border(GltkLabel* label, gboolean drawBorder)
{
	g_return_if_fail(GLTK_IS_LABEL(label));
	USING_PRIVATE(label);
	priv->drawBorder = drawBorder;
	gltk_widget_invalidate(GLTK_WIDGET(label));
}

void
gltk_label_set_font_size(GltkLabel* label, int size)
{
	g_return_if_fail(GLTK_IS_LABEL(label));
	USING_PRIVATE(label);
	priv->fontSize = size;
	gltk_widget_layout(GLTK_WIDGET(label));
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
	GltkGLFont* font = gltk_fonts_cache_get_font(GLTK_FONTS_BASE, priv->fontSize, FALSE);
	g_assert(font);

	size->height = 0;
	size->width = 0;
	
	GltkGLFontBounds bounds = gltk_fonts_measure_string(font, GLTK_LABEL(widget)->text);

	size->width = bounds.width;
	size->height = bounds.height;

	size->width += 20;
	size->height += 20;

	// g_message("Request: %i %i", size->width, size->height);

	GLTK_WIDGET_CLASS(gltk_label_parent_class)->size_request(widget, size);
}

static void
gltk_label_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);

	GltkLabel* label = GLTK_LABEL(widget);

	GltkAllocation allocation = gltk_widget_get_allocation(widget);
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
		GltkGLFont* font = gltk_fonts_cache_get_font(GLTK_FONTS_BASE, priv->fontSize, TRUE);
		glColor3f(label->color.r, label->color.g, label->color.b);
		glTranslatef(10.0f, font->ascender + font->descender + 10, 0.1f);
		glScalef(1.0f, -1.0f, 1.0f);

		gchar** lines = g_strsplit(label->text, "\n", -1);
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


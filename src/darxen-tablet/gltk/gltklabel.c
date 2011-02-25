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
}

static void
gltk_label_dispose(GObject* gobject)
{
	GltkLabel* self = GLTK_LABEL(gobject);
	USING_PRIVATE(self);

	//free and release references

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
	USING_PRIVATE(label);
	if (priv->text)
		g_free(priv->text);
	priv->text = g_strdup(text);
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
	size->width = 100;
	size->height = 50;
	GLTK_WIDGET_CLASS(gltk_label_parent_class)->size_request(widget, size);
}

static void
gltk_label_render(GltkWidget* label)
{
	USING_PRIVATE(label);

	GltkAllocation allocation = gltk_widget_get_allocation(GLTK_WIDGET(label));

	glBegin(GL_QUADS);
	{
		if (!g_strcmp0(priv->text, "red"))
			glColor3f(1.0f, 0.0f, 0.0f);
		else if (!g_strcmp0(priv->text, "green"))
			glColor3f(0.0f, 1.0f, 0.0f);
		else
			glColor3f(0.0f, 0.0f, 1.0f);

		glVertex2i(0, 0);
		glVertex2i(0, allocation.height);
		glVertex2i(allocation.width, allocation.height);
		glVertex2i(allocation.width, 0);
	}
	glEnd();
}


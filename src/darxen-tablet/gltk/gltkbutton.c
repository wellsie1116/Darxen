/* gltkbutton.c
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

#include "gltkbutton.h"

#include <string.h>

#include <GL/gl.h>

G_DEFINE_TYPE(GltkButton, gltk_button, GLTK_TYPE_WIDGET)

#define USING_PRIVATE(obj) GltkButtonPrivate* priv = GLTK_BUTTON_GET_PRIVATE(obj)
#define GLTK_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_BUTTON, GltkButtonPrivate))

enum
{
	CLICKED,
	LAST_SIGNAL
};

typedef struct _GltkButtonPrivate		GltkButtonPrivate;
struct _GltkButtonPrivate
{
	gchar* text;
};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_button_dispose(GObject* gobject);
static void gltk_button_finalize(GObject* gobject);

static void gltk_button_size_request(GltkWidget* widget, GltkSize* size);
static void gltk_button_render(GltkWidget* widget);

static void
gltk_button_class_init(GltkButtonClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkButtonPrivate));
	
	signals[CLICKED] = 
		g_signal_new(	"size-request",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkWidgetClass, size_request),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	
	gobject_class->dispose = gltk_button_dispose;
	gobject_class->finalize = gltk_button_finalize;

	gltkwidget_class->size_request = gltk_button_size_request;
	gltkwidget_class->render = gltk_button_render;
}

static void
gltk_button_init(GltkButton* self)
{
	USING_PRIVATE(self);

	priv->text = NULL;
}

static void
gltk_button_dispose(GObject* gobject)
{
	GltkButton* self = GLTK_BUTTON(gobject);
	USING_PRIVATE(self);

	//free and release references

	G_OBJECT_CLASS(gltk_button_parent_class)->dispose(gobject);
}

static void
gltk_button_finalize(GObject* gobject)
{
	GltkButton* self = GLTK_BUTTON(gobject);
	USING_PRIVATE(self);

	g_free(priv->text);

	G_OBJECT_CLASS(gltk_button_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_button_new(gchar* text)
{
	GObject *gobject = g_object_new(GLTK_TYPE_BUTTON, NULL);
	GltkButton* self = GLTK_BUTTON(gobject);

	USING_PRIVATE(self);

	priv->text = g_strdup(text);

	return (GltkWidget*)gobject;
}


GQuark
gltk_button_error_quark()
{
	return g_quark_from_static_string("gltk-button-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_button_size_request(GltkWidget* widget, GltkSize* size)
{
	USING_PRIVATE(widget);

	size->height = 50;
	size->width = strlen(priv->text) * 20;

	GLTK_WIDGET_CLASS(gltk_button_parent_class)->size_request(widget, size);
}

static void
gltk_button_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);

	GltkAllocation allocation = gltk_widget_get_allocation(GLTK_WIDGET(widget));

	glBegin(GL_QUADS);
	{
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2i(0, allocation.height);
		glVertex2i(allocation.width, allocation.height);
		glColor3f(0.0f, 1.0f, 1.0f);
		glVertex2i(allocation.width, 0);
		glVertex2i(0, 0);
	}
	glEnd();
}


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
	gboolean isDown;
};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_button_dispose(GObject* gobject);
static void gltk_button_finalize(GObject* gobject);

static void gltk_button_size_request(GltkWidget* widget, GltkSize* size);
static void gltk_button_render(GltkWidget* widget);
static gboolean gltk_button_click_event(GltkWidget* widget, GltkEventClick* touch);
static gboolean gltk_button_touch_event(GltkWidget* widget, GltkEventTouch* touch);

static void gltk_button_clicked(GltkWidget* widget);

static void
gltk_button_class_init(GltkButtonClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);
	GltkButtonClass* gltkbutton_class = GLTK_BUTTON_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkButtonPrivate));
	
	signals[CLICKED] = 
		g_signal_new(	"clicked",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkButtonClass, clicked),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	
	gobject_class->dispose = gltk_button_dispose;
	gobject_class->finalize = gltk_button_finalize;

	gltkwidget_class->size_request = gltk_button_size_request;
	gltkwidget_class->render = gltk_button_render;
	gltkwidget_class->click_event = gltk_button_click_event;
	gltkwidget_class->touch_event = gltk_button_touch_event;

	gltkbutton_class->clicked = gltk_button_clicked;
}

static void
gltk_button_init(GltkButton* self)
{
	USING_PRIVATE(self);

	priv->text = NULL;
	priv->isDown = FALSE;
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

const gchar*
gltk_button_get_text(GltkButton* button)
{
	USING_PRIVATE(button);
	
	return priv->text;
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
	g_message("Rendering");

	glBegin(GL_QUADS);
	{
		if (!priv->isDown)
			glColor3f(0.0f, 0.0f, 1.0f);
		else
			glColor3f(0.0f, 1.0f, 1.0f);
		glVertex2i(0, allocation.height);
		glVertex2i(allocation.width, allocation.height);
		if (priv->isDown)
			glColor3f(0.0f, 0.0f, 1.0f);
		else
			glColor3f(0.0f, 1.0f, 1.0f);
		glVertex2i(allocation.width, 0);
		glVertex2i(0, 0);
	}
	glEnd();
}

static gboolean
gltk_button_click_event(GltkWidget* widget, GltkEventClick* touch)
{
	g_message("A button has been truly clicked!!!!!");
	return TRUE;
}

static gboolean
gltk_button_touch_event(GltkWidget* widget, GltkEventTouch* touch)
{
	USING_PRIVATE(widget);

	if (touch->touchType == TOUCH_BEGIN)
	{
		priv->isDown = TRUE;
		gltk_window_set_widget_pressed(widget->window, widget);
	}
	else if (touch->touchType == TOUCH_END)
	{
		priv->isDown = FALSE;
		gltk_window_set_widget_unpressed(widget->window, widget);
	}
	else
	{
		return FALSE;
	}

	gltk_widget_invalidate(widget);

	return TRUE;
}

static void gltk_button_clicked(GltkWidget* widget)
{
	g_message("A button was clicked...yay");
}


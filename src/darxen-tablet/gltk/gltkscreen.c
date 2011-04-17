/* gltkscreen.c
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

#include "gltkscreen.h"

#include <GL/gl.h>
#include <glib.h>

G_DEFINE_TYPE(GltkScreen, gltk_screen, GLTK_TYPE_WIDGET)

#define USING_PRIVATE(obj) GltkScreenPrivate* priv = GLTK_SCREEN_GET_PRIVATE(obj)
#define GLTK_SCREEN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_SCREEN, GltkScreenPrivate))

enum
{
	RENDER_OVERLAY,

	LAST_SIGNAL
};

typedef struct _GltkScreenPrivate		GltkScreenPrivate;
struct _GltkScreenPrivate
{
	GltkWidget* widget;
	GltkWindow* window;
};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_screen_dispose(GObject* gobject);
static void gltk_screen_finalize(GObject* gobject);

static void gltk_screen_real_layout(GltkScreen* screen);
static void gltk_screen_size_request(GltkWidget* widget, GltkSize* size);
static void gltk_screen_size_allocate(GltkWidget* widget, GltkAllocation* allocation);
static gboolean gltk_screen_event(GltkWidget* widget, GltkEvent* event);
static void gltk_screen_render(GltkWidget* widget);

static void
gltk_screen_class_init(GltkScreenClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkScreenPrivate));
	
	signals[RENDER_OVERLAY] = 
		g_signal_new(	"render-overlay",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkScreenClass, render_overlay),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	
	
	gobject_class->dispose = gltk_screen_dispose;
	gobject_class->finalize = gltk_screen_finalize;
	
	gltkwidget_class->size_request = gltk_screen_size_request;
	gltkwidget_class->size_allocate = gltk_screen_size_allocate;
	gltkwidget_class->event = gltk_screen_event;
	gltkwidget_class->render = gltk_screen_render;

	klass->layout = gltk_screen_real_layout;
}

static void
gltk_screen_init(GltkScreen* self)
{
	USING_PRIVATE(self);

	self->maximized = TRUE;

	priv->widget = NULL;
}

static void
gltk_screen_dispose(GObject* gobject)
{
	GltkScreen* self = GLTK_SCREEN(gobject);
	USING_PRIVATE(self);

	if (priv->widget)
	{
		g_object_unref(G_OBJECT(priv->widget));
		priv->widget = NULL;
	}

	G_OBJECT_CLASS(gltk_screen_parent_class)->dispose(gobject);
}

static void
gltk_screen_finalize(GObject* gobject)
{
	//GltkScreen* self = GLTK_SCREEN(gobject);
	//USING_PRIVATE(self);

	//free memory

	G_OBJECT_CLASS(gltk_screen_parent_class)->finalize(gobject);
}

GltkScreen*
gltk_screen_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_SCREEN, NULL);
	//GltkScreen* self = GLTK_SCREEN(gobject);

	//USING_PRIVATE(self);

	return (GltkScreen*)gobject;
}

void
gltk_screen_set_window(GltkScreen* screen, GltkWindow* window)
{
 	g_return_if_fail(GLTK_IS_SCREEN(screen));
	g_return_if_fail(!window || GLTK_IS_WINDOW(window));
	USING_PRIVATE(screen);

	priv->window = window;
}

void
gltk_screen_set_root(GltkScreen* screen, GltkWidget* widget)
{
 	g_return_if_fail(GLTK_IS_SCREEN(screen));
	g_return_if_fail(GLTK_IS_WIDGET(widget));
 
 	USING_PRIVATE(screen);
 
	g_object_ref(G_OBJECT(widget));

	priv->widget = widget;

	gltk_widget_set_screen(widget, screen);
	gltk_widget_set_parent(widget, GLTK_WIDGET(screen));
 
	if (priv->window)
		gltk_window_invalidate(priv->window);
}

GltkSize
gltk_screen_get_window_size(GltkScreen* screen)
{
	GltkSize res = {0,};
	g_return_val_if_fail(GLTK_IS_SCREEN(screen), res);
	USING_PRIVATE(screen);
	g_return_val_if_fail(priv->window, res);

	return gltk_window_get_size(priv->window);
}

void
gltk_screen_push_screen(GltkScreen* screen, GltkScreen* newScreen)
{
	g_return_if_fail(GLTK_IS_SCREEN(screen));
	USING_PRIVATE(screen);
	g_return_if_fail(priv->window);

	gltk_window_push_screen(priv->window, newScreen);
}

void
gltk_screen_pop_screen(GltkScreen* screen, GltkScreen* newScreen)
{
	g_return_if_fail(GLTK_IS_SCREEN(screen));
	USING_PRIVATE(screen);
	g_return_if_fail(priv->window);

	gltk_window_pop_screen(priv->window, newScreen);
}

void
gltk_screen_close_window(GltkScreen* screen)
{
	g_return_if_fail(GLTK_IS_SCREEN(screen));
	USING_PRIVATE(screen);
	g_return_if_fail(priv->window);
	
	gltk_window_close(priv->window);
}

void
gltk_screen_invalidate(GltkScreen* screen)
{
	g_return_if_fail(GLTK_IS_SCREEN(screen));
	USING_PRIVATE(screen);
	g_return_if_fail(priv->window);

	gltk_window_invalidate(priv->window);
}

void
gltk_screen_layout(GltkScreen* screen)
{
	g_return_if_fail(GLTK_IS_SCREEN(screen));

	GLTK_SCREEN_GET_CLASS(screen)->layout(screen);

	gltk_screen_invalidate(screen);
}

gboolean
gltk_screen_set_widget_pressed(GltkScreen* screen, GltkWidget* widget)
{
	g_return_val_if_fail(GLTK_IS_SCREEN(screen), FALSE);
	USING_PRIVATE(screen);
	g_return_val_if_fail(priv->window, FALSE);

	return gltk_window_set_widget_pressed(priv->window, widget);
}

void
gltk_screen_swap_widget_pressed(GltkScreen* screen, GltkWidget* before, GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_SCREEN(screen));
	USING_PRIVATE(screen);
	g_return_if_fail(priv->window);

	gltk_window_swap_widget_pressed(priv->window, before, widget);
}

void
gltk_screen_set_widget_unpressed(GltkScreen* screen, GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_SCREEN(screen));
	USING_PRIVATE(screen);
	g_return_if_fail(priv->window);

	gltk_window_set_widget_unpressed(priv->window, widget);
}

GltkWidget*
gltk_screen_find_drop_target(GltkScreen* screen, const gchar* type, GltkRectangle* bounds)
{
	g_return_val_if_fail(GLTK_IS_SCREEN(screen), NULL);
	USING_PRIVATE(screen);
	g_return_val_if_fail(priv->widget, NULL);

	return gltk_widget_find_drop_target(priv->widget, type, bounds);
}


GQuark
gltk_screen_error_quark()
{
	return g_quark_from_static_string("gltk-screen-error-quark");
}

/*********************
 * Private Functions *
 *********************/


static void
gltk_screen_real_layout(GltkScreen* screen)
{
	g_return_if_fail(GLTK_IS_SCREEN(screen));
	USING_PRIVATE(screen);
	g_return_if_fail(priv->widget);

	GltkAllocation myAllocation = gltk_widget_get_allocation(GLTK_WIDGET(screen));

	GltkAllocation allocation = {0, 0, myAllocation.width, myAllocation.height};
	gltk_widget_size_allocate(priv->widget, allocation);
}

static void
gltk_screen_size_request(GltkWidget* widget, GltkSize* size)
{
	USING_PRIVATE(widget);
	
	if (!priv->widget)
	{
		size->width = 0;
		size->height = 0;
	}
	else
	{
		gltk_widget_size_request(priv->widget, size);
	}
	
	GLTK_WIDGET_CLASS(gltk_screen_parent_class)->size_request(widget, size);
}

static gboolean
gltk_screen_event(GltkWidget* widget, GltkEvent* event)
{
	USING_PRIVATE(widget);

	gboolean returnValue = FALSE;

	if (priv->widget)
	{
		GltkEvent* transformed = gltk_event_clone(event);
		//gltk_screen_transform_event(GLTK_SCREEN(widget), transformed);
		returnValue = gltk_widget_send_event(priv->widget, transformed);
		gltk_event_free(transformed);
	}

	return returnValue;
}

static void
gltk_screen_size_allocate(GltkWidget* widget, GltkAllocation* allocation)
{
	GLTK_WIDGET_CLASS(gltk_screen_parent_class)->size_allocate(widget, allocation);

	gltk_screen_layout(GLTK_SCREEN(widget));
}

static void
gltk_screen_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);

	if (!priv->widget)
		return;

	if (GLTK_SCREEN(widget)->maximized)
	{
		GltkAllocation allocation = gltk_widget_get_allocation(widget);
		glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
		glRectf(allocation.x, allocation.y, allocation.x + allocation.width, allocation.y + allocation.height);
	}

	gltk_widget_render(priv->widget);
	
	g_signal_emit(G_OBJECT(widget), signals[RENDER_OVERLAY], 0);
}

GltkWidget*
gltk_screen_get_root(GltkScreen* screen)
{
	USING_PRIVATE(screen);

	return priv->widget;
}


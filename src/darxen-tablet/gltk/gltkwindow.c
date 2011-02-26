/* gltkwindow.c
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

#include "gltkwindow.h"

#include "gltkwidget.h"

G_DEFINE_TYPE(GltkWindow, gltk_window, G_TYPE_OBJECT)

#define USING_PRIVATE(obj) GltkWindowPrivate* priv = GLTK_WINDOW_GET_PRIVATE(obj)
#define GLTK_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_WINDOW, GltkWindowPrivate))

static void gltk_window_dispose(GObject* gobject);
static void gltk_window_finalize(GObject* gobject);

static void	gltk_window_press_complete(GltkWindow* window);

typedef struct _GltkWindowPrivate		GltkWindowPrivate;
struct _GltkWindowPrivate
{
	int width;
	int height;

	GltkWidget* root;

	GltkWidget* pressed;
	GltkWidget* unpressed;

	GltkWindowCallbacks callbacks;
};

static void
gltk_window_class_init(GltkWindowClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkWindowPrivate));
	
	gobject_class->dispose = gltk_window_dispose;
	gobject_class->finalize = gltk_window_finalize;
}

static void
gltk_window_init(GltkWindow* self)
{
	USING_PRIVATE(self);

	static GltkWindowCallbacks emptyCallbacks = {0,};

	priv->width = -1;
	priv->height = -1;
	priv->root = NULL;
	priv->callbacks = emptyCallbacks;
}

static void
gltk_window_dispose(GObject* gobject)
{
	GltkWindow* self = GLTK_WINDOW(gobject);
	USING_PRIVATE(self);

	if (priv->root)
	{
		g_object_unref(priv->root);
		priv->root = NULL;
	}

	G_OBJECT_CLASS(gltk_window_parent_class)->dispose(gobject);
}

static void
gltk_window_finalize(GObject* gobject)
{
	GltkWindow* self = GLTK_WINDOW(gobject);
	USING_PRIVATE(self);

	G_OBJECT_CLASS(gltk_window_parent_class)->finalize(gobject);
}

GltkWindow*
gltk_window_new(GltkWindowCallbacks callbacks)
{
	GObject *gobject = g_object_new(GLTK_TYPE_WINDOW, NULL);
	GltkWindow* self = GLTK_WINDOW(gobject);

	USING_PRIVATE(self);
	priv->callbacks = callbacks;

	return (GltkWindow*)gobject;
}

void
gltk_window_set_size(GltkWindow* window, int width, int height)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));

	USING_PRIVATE(window);
	priv->width = width;
	priv->height = height;
	
	if (priv->root)
	{
		GltkAllocation allocation = {0, 0, width, height};
		gltk_widget_size_allocate(priv->root, allocation);
	}

	if (priv->callbacks.request_render)
		priv->callbacks.request_render();
}

void
gltk_window_render(GltkWindow* window)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));

	USING_PRIVATE(window);

	g_return_if_fail(priv->root);
	
	gltk_widget_render(priv->root);
}

gboolean
gltk_window_send_event(GltkWindow* window, GltkEvent* event)
{
	g_return_val_if_fail(GLTK_IS_WINDOW(window), FALSE);

	USING_PRIVATE(window);

	g_return_val_if_fail(priv->root, FALSE);

	gboolean returnValue;

	if (event->type == GLTK_TOUCH && event->touch.touchType == TOUCH_MOVE && priv->pressed)
	{
		//redirect touch move event to pressed widget
		GltkEvent* e = gltk_event_clone(event);
		GltkWidget* parent = priv->pressed;
		for (parent = priv->pressed; parent; parent = gltk_widget_get_parent(parent))
		{
			GltkAllocation allocation = gltk_widget_get_allocation(parent);
			int i;
			for (i = 0; i < e->touch.fingers; i++)
			{
				e->touch.positions[i].x -= allocation.x;
				e->touch.positions[i].y -= allocation.y;
			}
		}
		returnValue = gltk_widget_send_event(priv->pressed, e);
		gltk_event_free(e);
	}
	else
	{
		returnValue = gltk_widget_send_event(priv->root, event);
	}

	//check to spawn a click event
	if (event->touch.touchType == TOUCH_END)
		gltk_window_press_complete(window);

	return returnValue;
}


void
gltk_window_set_root(GltkWindow* window, GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));
	g_return_if_fail(GLTK_IS_WIDGET(widget));

	USING_PRIVATE(window);

	g_object_ref(G_OBJECT(widget));

	priv->root = widget;
	gltk_widget_set_window(widget, window);

	if (priv->callbacks.request_render)
		priv->callbacks.request_render();
}

void
gltk_window_invalidate(GltkWindow* window)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));
	USING_PRIVATE(window);

	if (priv->callbacks.request_render)
		priv->callbacks.request_render();
}

void
gltk_window_set_widget_pressed(GltkWindow* window, GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));
	g_return_if_fail(GLTK_IS_WIDGET(widget));
	USING_PRIVATE(window);

	//FIXME: press events can be stacked, this is odd
	g_assert(!priv->pressed);
	g_object_ref(G_OBJECT(widget));
	priv->pressed = widget;

	//g_object_connect(G_OBJECT(widget), "event", pressed_widget_event
}

void
gltk_window_set_widget_unpressed(GltkWindow* window, GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));
	g_return_if_fail(GLTK_IS_WIDGET(widget));
	USING_PRIVATE(window);
	
	if (priv->unpressed)
		return;

	g_object_ref(G_OBJECT(widget));
	priv->unpressed = widget;
}



GQuark
gltk_window_error_quark()
{
	return g_quark_from_static_string("gltk-window-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_window_press_complete(GltkWindow* window)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));
	USING_PRIVATE(window);
	
	if (priv->pressed && (priv->pressed == priv->unpressed))
	{
		//spawn a clicked event
		GltkEvent* event = gltk_event_new(GLTK_CLICK);
		
		gltk_widget_send_event(priv->pressed, event);
	}
	else if (priv->pressed)
	{
		//inject the lost TOUCH_END event
		GltkEvent* event = gltk_event_new(GLTK_TOUCH);
		event->touch.touchType = TOUCH_END;
		event->touch.fingers = 1;
		event->touch.positions = g_new(GltkTouchPosition, 1);
		event->touch.positions->x = -1;
		event->touch.positions->y = -1;
		gltk_widget_send_event(priv->pressed, event);
	}

	if (priv->pressed)
	{
		g_object_unref(G_OBJECT(priv->pressed));
		priv->pressed = NULL;
	}
	if (priv->unpressed)
	{
		g_object_unref(G_OBJECT(priv->unpressed));
		priv->unpressed = NULL;
	}
}


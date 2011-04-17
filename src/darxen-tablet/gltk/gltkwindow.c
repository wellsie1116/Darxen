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
#include "gltkscreen.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include <math.h>

G_DEFINE_TYPE(GltkWindow, gltk_window, G_TYPE_OBJECT)

#define USING_PRIVATE(obj) GltkWindowPrivate* priv = GLTK_WINDOW_GET_PRIVATE(obj)
#define GLTK_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_WINDOW, GltkWindowPrivate))
enum
{
	REQUEST_RENDER,
	CLOSE,
   	LAST_SIGNAL
};

static void gltk_window_dispose(GObject* gobject);
static void gltk_window_finalize(GObject* gobject);

static gboolean	gltk_window_press_complete(GltkWindow* window);

typedef struct _GltkWindowPrivate		GltkWindowPrivate;
struct _GltkWindowPrivate
{
	int width;
	int height;

	gboolean rendered;

	GQueue* screens;

	GltkTouchPosition pressedPosition;
	guint longPressPending;
	gboolean longPressed;
	GltkWidget* pressed;
	GltkWidget* unpressed;

	gboolean touchCount;
	GltkTouchPosition* touchPositions;
};

static guint signals[LAST_SIGNAL] = {0,};

static void
gltk_window_class_init(GltkWindowClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkWindowPrivate));
	
	signals[REQUEST_RENDER] = 
		g_signal_new(	"request-render",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkWindowClass, request_render),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	
	signals[CLOSE] = 
		g_signal_new(	"close",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkWindowClass, close),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	
	gobject_class->dispose = gltk_window_dispose;
	gobject_class->finalize = gltk_window_finalize;
}

static void
gltk_window_init(GltkWindow* self)
{
	USING_PRIVATE(self);

	priv->width = -1;
	priv->height = -1;
	priv->rendered = FALSE;
	priv->screens = NULL;
	priv->longPressPending = 0;
	priv->pressed = NULL;
	priv->unpressed = NULL;

	priv->touchCount = 0;
}

static void
gltk_window_dispose(GObject* gobject)
{
	GltkWindow* self = GLTK_WINDOW(gobject);
	USING_PRIVATE(self);

	if (priv->screens)
	{
		GltkScreen* screen;
		while ((screen = g_queue_pop_head(priv->screens)))
		{
			gltk_screen_set_window(screen, NULL);
			gltk_widget_unparent(GLTK_WIDGET(screen));
			g_object_unref(G_OBJECT(screen));
		}
		g_queue_free(priv->screens);
		priv->screens = NULL;
	}

	G_OBJECT_CLASS(gltk_window_parent_class)->dispose(gobject);
}

static void
gltk_window_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_window_parent_class)->finalize(gobject);
}

GltkWindow*
gltk_window_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_WINDOW, NULL);
	USING_PRIVATE(gobject);

	priv->screens = g_queue_new();

	return (GltkWindow*)gobject;
}

void
gltk_window_set_size(GltkWindow* window, int width, int height)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));

	USING_PRIVATE(window);
	priv->width = width;
	priv->height = height;
	
	gltk_window_layout(window);

	g_signal_emit(G_OBJECT(window), signals[REQUEST_RENDER], 0);
}

GltkSize
gltk_window_get_size(GltkWindow* window)
{
	GltkSize size = {-1, -1};
	g_return_val_if_fail(GLTK_IS_WINDOW(window), size);

	USING_PRIVATE(window);

	size.width = priv->width;
	size.height = priv->height;

	return size;
}

void
layout_screen(GltkScreen* screen, GltkWindow* window)
{
	USING_PRIVATE(window);
	if (priv->width <= 0 || priv->height <= 0)
		return;

	if (screen->maximized)
	{
		//fill the window
		GltkAllocation allocation = {0, 0, priv->width, priv->height};
		gltk_widget_size_allocate(GLTK_WIDGET(screen), allocation);
	}
	else
	{
		//center the screen in the window
		GltkSize size;
		gltk_widget_size_request(GLTK_WIDGET(screen), &size);

		int dw = MAX(0, priv->width - size.width);
		int dh = MAX(0, priv->height - size.height);
		GltkAllocation allocation = {dw/2, dh/2, priv->width - dw, priv->height - dh};
		gltk_widget_size_allocate(GLTK_WIDGET(screen), allocation);
	}
}

void
gltk_window_layout(GltkWindow* window)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));

	USING_PRIVATE(window);

	if (priv->screens)
	{
		g_queue_foreach(priv->screens, (GFunc)layout_screen, window);
	}
}

void
gltk_window_render(GltkWindow* window)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));

	USING_PRIVATE(window);

	GList* pScreens = priv->screens->head;
	while (pScreens)
	{
		GltkScreen* screen = (GltkScreen*)pScreens->data;

		glLineWidth(1.0f);

		//TODO check if windows are obscured before rendering them
		//How should transparency be handled?
		gltk_widget_render(GLTK_WIDGET(screen));

		pScreens = pScreens->next;
	}

	//render touch points
	if (priv->touchCount)
	{
		glColor3f(0.2f, 0.2f, 1.0f);
		GLUquadric* quadric = gluNewQuadric();
		int i;
		for (i = 0; i < priv->touchCount; i++)
		{
			//g_message("Touch position (%i): %i %i", i, priv->touchPositions[i].x, priv->touchPositions[i].y);
			glPushMatrix();
			{
				glTranslatef(priv->touchPositions[i].x, priv->touchPositions[i].y, 0.0f);
				gluDisk(quadric, 5.0, 20.0, 15, 5);
			}
			glPopMatrix();
		}
		gluDeleteQuadric(quadric);
	}
	if (!priv->rendered)
	{
		priv->rendered = TRUE;
		gltk_window_layout(window);
	}
}

gboolean
gltk_window_send_event(GltkWindow* window, GltkEvent* event)
{
	g_return_val_if_fail(GLTK_IS_WINDOW(window), FALSE);

	USING_PRIVATE(window);

	g_return_val_if_fail(g_queue_get_length(priv->screens), FALSE);

	gboolean returnValue = FALSE;

	if (event->type == GLTK_TOUCH)
	{
		if (priv->touchPositions)
			g_free(priv->touchPositions);
		priv->touchPositions = g_new(GltkTouchPosition, 1);
		*priv->touchPositions = *event->touch.positions;
		switch (event->touch.touchType)
		{
			case TOUCH_BEGIN:
				priv->touchCount = 1;
				break;
			case TOUCH_END:
				priv->touchCount = 0;
				break;
			default:
				break;
		}
		gltk_window_invalidate(window);
	}
	else if (event->type == GLTK_MULTI_DRAG)
	{
		if (priv->touchPositions)
			g_free(priv->touchPositions);
		priv->touchPositions = g_new(GltkTouchPosition, event->multidrag.fingers);
		int i;
		for (i = 0; i < event->multidrag.fingers; i++)
			priv->touchPositions[i] = event->multidrag.positions[i];
		priv->touchCount = event->multidrag.fingers;
		gltk_window_invalidate(window);
	}
	else if (event->type == GLTK_PINCH)
	{
		if (priv->touchPositions)
			g_free(priv->touchPositions);
		priv->touchPositions = g_new(GltkTouchPosition, event->pinch.fingers);
		int i;
		for (i = 0; i < event->pinch.fingers; i++)
			priv->touchPositions[i] = event->pinch.positions[i];
		priv->touchCount = event->pinch.fingers;
		gltk_window_invalidate(window);
	}
	else if (event->type == GLTK_ROTATE)
	{
		if (priv->touchPositions)
			g_free(priv->touchPositions);
		priv->touchPositions = g_new(GltkTouchPosition, event->rotate.fingers);
		int i;
		for (i = 0; i < event->rotate.fingers; i++)
			priv->touchPositions[i] = event->rotate.positions[i];
		priv->touchCount = event->rotate.fingers;
		gltk_window_invalidate(window);
	}

	if (event->type == GLTK_TOUCH && event->touch.touchType == TOUCH_BEGIN)
		priv->pressedPosition = *event->touch.positions;

	if (event->type == GLTK_TOUCH && event->touch.touchType == TOUCH_MOVE && priv->pressed)
	{
		int dx = event->touch.positions->x - priv->pressedPosition.x;
		int dy = event->touch.positions->y - priv->pressedPosition.y;

		//store current translation
		if (priv->longPressPending)
		{
			float dist = sqrt(dx*dx+dy*dy);
			if (dist > 10.0f)
			{
				g_source_remove(priv->longPressPending);
				priv->longPressPending = 0;
			}
			
			//TODO: also, send a touch move event?
		}

		if (!priv->longPressPending)
		{
			//send a drag event to the pressed widget
			GltkEvent* e = gltk_event_new(GLTK_DRAG);
			e->drag.dx = dx;
			e->drag.dy = dy;
			e->drag.longTouched = priv->longPressed;
			priv->pressedPosition = *(event->touch.positions);

			returnValue = gltk_widget_send_event(priv->pressed, e);
			gltk_event_free(e);

			if (!returnValue)
			{
				//instead, redirect touch move event to pressed widget
				e = gltk_event_clone(event);
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
		}
	}
	//else if (event->type == GLTK_TOUCH && event->touch.touchType == TOUCH_END && priv->pressed)
	//{
	//	//do not send the event, they aren't expecting it
	//}
	else
	{
		GList* pScreens = priv->screens->tail;
		while (pScreens && !returnValue)
		{
			GltkScreen* screen = (GltkScreen*)pScreens->data;

			returnValue = gltk_widget_send_event(GLTK_WIDGET(screen), event) || screen->maximized;

			pScreens = pScreens->prev;

			//TODO: some amount of collision detection should be used to determine if an event was
			//ignored instead of not hitting the window
			break;
		}
	}

	//check to spawn a click event
	if (event->touch.touchType == TOUCH_END)
		returnValue = gltk_window_press_complete(window) || returnValue;

	return returnValue;
}

void
gltk_window_push_screen(GltkWindow* window, GltkScreen* screen)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));
	g_return_if_fail(GLTK_IS_SCREEN(screen));

	USING_PRIVATE(window);

	g_object_ref(screen);

	g_queue_push_tail(priv->screens, screen);

	gltk_screen_set_window(screen, window);
	gltk_widget_set_screen(GLTK_WIDGET(screen), screen);
	layout_screen(screen, window);

	g_signal_emit(G_OBJECT(window), signals[REQUEST_RENDER], 0);
}

void
gltk_window_pop_screen(GltkWindow* window, GltkScreen* screen)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));
	g_return_if_fail(!screen || GLTK_IS_SCREEN(screen));

	USING_PRIVATE(window);

	if (!g_queue_get_length(priv->screens))
	{
		g_critical("Popping from an empty stack of screens");
		return;
	}

	if (!screen)
	{
		screen = g_queue_pop_tail(priv->screens);
	}
	else
	{
		if (!g_queue_find(priv->screens, screen))
		{
			g_critical("Screen not in stack of screens");
			return;
		}

		g_queue_remove(priv->screens, screen);
	}

	gltk_screen_set_window(screen, NULL);
	gltk_widget_set_screen(GLTK_WIDGET(screen), NULL);
	g_object_unref(screen);

	g_signal_emit(G_OBJECT(window), signals[REQUEST_RENDER], 0);
}

void
gltk_window_close(GltkWindow* window)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));

	g_signal_emit(G_OBJECT(window), signals[CLOSE], 0);
}

void
gltk_window_invalidate(GltkWindow* window)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));

	g_signal_emit(G_OBJECT(window), signals[REQUEST_RENDER], 0);
}

static gboolean
check_long_press(GltkWindow* window)
{
	USING_PRIVATE(window);

	priv->longPressPending = 0;
	priv->longPressed = TRUE;

	//spawn long press event
	GltkEvent* event = gltk_event_new(GLTK_LONG_TOUCH);
	gltk_widget_send_event(priv->pressed, event);
	gltk_event_free(event);

	return FALSE;
}

gboolean
gltk_window_set_widget_pressed(GltkWindow* window, GltkWidget* widget)
{
	g_return_val_if_fail(GLTK_IS_WINDOW(window), FALSE);
	g_return_val_if_fail(GLTK_IS_WIDGET(widget), FALSE);
	USING_PRIVATE(window);

	if (priv->pressed)
		return FALSE;

	priv->pressed = widget;
	g_object_add_weak_pointer(G_OBJECT(widget), (gpointer*)&priv->pressed);
	priv->longPressed = FALSE;

	if (priv->longPressPending)
		g_source_remove(priv->longPressPending);
	priv->longPressPending = g_timeout_add(1000, (GSourceFunc)check_long_press, window);
	return TRUE;
}

void
gltk_window_swap_widget_pressed(GltkWindow* window, GltkWidget* before, GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_WINDOW(window));
	g_return_if_fail(GLTK_IS_WIDGET(before));
	g_return_if_fail(GLTK_IS_WIDGET(widget));
	USING_PRIVATE(window);

	g_return_if_fail(before == priv->pressed);

	g_object_remove_weak_pointer(G_OBJECT(priv->pressed), (gpointer*)&priv->pressed);
	priv->pressed = widget;
	g_object_add_weak_pointer(G_OBJECT(widget), (gpointer*)&priv->pressed);
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

static gboolean
gltk_window_press_complete(GltkWindow* window)
{
	g_return_val_if_fail(GLTK_IS_WINDOW(window), FALSE);
	USING_PRIVATE(window);

	gboolean returnValue = FALSE;
	
	if (priv->longPressPending)
	{
		g_source_remove(priv->longPressPending);
		priv->longPressPending = 0;
	}
	
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
		returnValue = gltk_widget_send_event(priv->pressed, event);
	}

	if (priv->pressed)
	{
		g_object_remove_weak_pointer(G_OBJECT(priv->pressed), (gpointer*)&priv->pressed);
		priv->pressed = NULL;
	}
	if (priv->unpressed)
	{
		g_object_unref(G_OBJECT(priv->unpressed));
		priv->unpressed = NULL;
	}

	return returnValue;
}


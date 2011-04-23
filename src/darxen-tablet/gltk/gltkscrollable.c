/* gltkscrollable.c
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

#include "gltkscrollable.h"

#include "gltkscreen.h"

#include <glib.h>
#include <GL/gl.h>

G_DEFINE_TYPE(GltkScrollable, gltk_scrollable, GLTK_TYPE_WIDGET)

#define USING_PRIVATE(obj) GltkScrollablePrivate* priv = GLTK_SCROLLABLE_GET_PRIVATE(obj)
#define GLTK_SCROLLABLE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_SCROLLABLE, GltkScrollablePrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkScrollablePrivate		GltkScrollablePrivate;
struct _GltkScrollablePrivate
{
	GltkWidget* widget;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void gltk_scrollable_dispose(GObject* gobject);
static void gltk_scrollable_finalize(GObject* gobject);

static void gltk_scrollable_size_request(GltkWidget* widget, GltkSize* size);
static void gltk_scrollable_size_allocate(GltkWidget* widget, GltkAllocation* allocation);
static gboolean gltk_scrollable_event(GltkWidget* widget, GltkEvent* event);
static GltkWidget* gltk_scrollable_find_drop_target(GltkWidget* widget, const gchar* type, const GltkRectangle* bounds);
static void gltk_scrollable_render(GltkWidget* widget);
static void gltk_scrollable_set_screen(GltkWidget* widget, GltkScreen* screen);
static gboolean gltk_scrollable_touch_event(GltkWidget* widget, GltkEventTouch* event);
static gboolean gltk_scrollable_drag_event(GltkWidget* widget, GltkEventDrag* event);

static void
gltk_scrollable_class_init(GltkScrollableClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkScrollablePrivate));
	
	gobject_class->dispose = gltk_scrollable_dispose;
	gobject_class->finalize = gltk_scrollable_finalize;

	gltkwidget_class->size_request = gltk_scrollable_size_request;
	gltkwidget_class->size_allocate = gltk_scrollable_size_allocate;
	gltkwidget_class->event = gltk_scrollable_event;
	gltkwidget_class->find_drop_target = gltk_scrollable_find_drop_target;
	gltkwidget_class->render = gltk_scrollable_render;
	gltkwidget_class->set_screen = gltk_scrollable_set_screen;
	gltkwidget_class->touch_event = gltk_scrollable_touch_event;
	gltkwidget_class->drag_event = gltk_scrollable_drag_event;
}

static void
gltk_scrollable_init(GltkScrollable* self)
{
	USING_PRIVATE(self);

	priv->widget = NULL;
	self->offset.x = 0;
	self->offset.y = 0;

	self->paddingTop = 0;
	self->paddingLeft = 0;
	self->paddingRight = 0;
	self->paddingBottom = 0;
}

static void
gltk_scrollable_dispose(GObject* gobject)
{
	GltkScrollable* self = GLTK_SCROLLABLE(gobject);
	USING_PRIVATE(self);

	if (priv->widget)
	{
		gltk_widget_unparent(priv->widget);
		gltk_widget_set_screen(priv->widget, NULL);
		g_object_unref(G_OBJECT(priv->widget));
		priv->widget = NULL;
	}

	G_OBJECT_CLASS(gltk_scrollable_parent_class)->dispose(gobject);
}

static void
gltk_scrollable_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_scrollable_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_scrollable_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_SCROLLABLE, NULL);

	return (GltkWidget*)gobject;
}

void
gltk_scrollable_set_widget(GltkScrollable* scrollable, GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_SCROLLABLE(scrollable));
	USING_PRIVATE(scrollable);

	if (priv->widget)
	{
		gltk_widget_unparent(priv->widget);
		gltk_widget_set_screen(priv->widget, NULL);
		g_object_unref(G_OBJECT(priv->widget));
	}

	g_object_ref_sink(G_OBJECT(widget));
	priv->widget = widget;
	scrollable->offset.x = 0;
	scrollable->offset.y = 0;
	gltk_widget_set_parent(widget, GLTK_WIDGET(scrollable));
	if (GLTK_WIDGET(scrollable)->screen)
		gltk_widget_set_screen(widget, GLTK_WIDGET(scrollable)->screen);
}

void
gltk_scrollable_transform_event(GltkScrollable* scrollable, GltkEvent* event)
{
	GltkAllocation allocation = gltk_widget_get_allocation(GLTK_WIDGET(scrollable));
	int i;

	switch (event->type)
	{
		case GLTK_TOUCH:
			for (i = 0; i < event->touch.fingers; i++)
			{
				event->touch.positions[i].x -= allocation.x;
				event->touch.positions[i].y -= allocation.y;
			}
			break;
		case GLTK_MULTI_DRAG:
			event->multidrag.center.x -= allocation.x;
			event->multidrag.center.y -= allocation.y;
			break;
		case GLTK_PINCH:
			event->pinch.center.x -= allocation.x;
			event->pinch.center.y -= allocation.y;
			break;
		case GLTK_ROTATE:
			event->rotate.center.x -= allocation.x;
			event->rotate.center.y -= allocation.y;
			break;
		default:
			break;
	}
}


GQuark
gltk_scrollable_error_quark()
{
	return g_quark_from_static_string("gltk-scrollable-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_scrollable_size_request(GltkWidget* widget, GltkSize* size)
{
	USING_PRIVATE(widget);
	
	if (!priv->widget)
	{
		size->width = 0;
		size->height = 0;
		return;
	}

	gltk_widget_size_request(priv->widget, size);

	GLTK_WIDGET_CLASS(gltk_scrollable_parent_class)->size_request(widget, size);
}

static gboolean
gltk_scrollable_event(GltkWidget* widget, GltkEvent* event)
{
	USING_PRIVATE(widget);

	gboolean returnValue = FALSE;

	if (priv->widget)
	{
		GltkEvent* transformed = gltk_event_copy(event);
		gltk_scrollable_transform_event(GLTK_SCROLLABLE(widget), transformed);
		returnValue = gltk_widget_send_event(priv->widget, transformed);
		gltk_event_free(transformed);
	}

	return returnValue;
}

static GltkWidget*
gltk_scrollable_find_drop_target(GltkWidget* widget, const gchar* type, const GltkRectangle* bounds)
{
	USING_PRIVATE(widget);

	if (priv->widget)
	{
		GltkWidget* target = gltk_widget_find_drop_target(priv->widget, type, bounds);
		if (target)
			return target;
	}

	return GLTK_WIDGET_CLASS(gltk_scrollable_parent_class)->find_drop_target(widget, type, bounds);
}

static void
gltk_scrollable_size_allocate(GltkWidget* widget, GltkAllocation* allocation)
{
	USING_PRIVATE(widget);

	if (priv->widget)
	{
		GltkSize size;
		gltk_widget_size_request(priv->widget, &size);
		GltkAllocation childAllocation = {GLTK_SCROLLABLE(widget)->offset.x, GLTK_SCROLLABLE(widget)->offset.y, MAX(allocation->width, size.width), MAX(allocation->height, size.height)};
		gltk_widget_size_allocate(priv->widget, childAllocation);
	}

	GLTK_WIDGET_CLASS(gltk_scrollable_parent_class)->size_allocate(widget, allocation);
}

static void
gltk_scrollable_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);

	if (!priv->widget)
		return;

	GltkAllocation allocation = gltk_widget_get_global_allocation(widget);
	GltkSize size = gltk_screen_get_window_size(widget->screen);

	//setup our rendering window how we like it
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int offsetX = allocation.x;
	int offsetY = size.height - allocation.height - allocation.y;
	//FIXME: why does the toplevel allocation cause an issue?
	{
		GltkWidget* toplevel = widget;
		GltkWidget* temp;
		while ((temp = gltk_widget_get_parent(toplevel)))
			toplevel = temp;
		GltkAllocation topAllocation = gltk_widget_get_allocation(toplevel);
		offsetX -= topAllocation.x;
		offsetY += topAllocation.y;
	}
	glViewport(offsetX, offsetY, allocation.width, allocation.height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, allocation.width, allocation.height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	{
		glLoadIdentity();
		glTranslatef(GLTK_SCROLLABLE(widget)->offset.x, GLTK_SCROLLABLE(widget)->offset.y, 0.0f);
		gltk_widget_render(priv->widget);

	}
	glPopMatrix();

	//undo our changes to the state
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

static void
gltk_scrollable_set_screen(GltkWidget* widget, GltkScreen* screen)
{
	USING_PRIVATE(widget);

	gltk_widget_set_screen(priv->widget, screen);

	GLTK_WIDGET_CLASS(gltk_scrollable_parent_class)->set_screen(widget, screen);
}

static gboolean
gltk_scrollable_touch_event(GltkWidget* widget, GltkEventTouch* event)
{
	if (event->touchType == TOUCH_BEGIN)
	{
		gltk_screen_set_widget_pressed(widget->screen, widget);
	}
	else if (event->touchType == TOUCH_END)
	{
		gltk_screen_set_widget_unpressed(widget->screen, widget);
	}
	else
	{
		return FALSE;
	}

	gltk_widget_invalidate(widget);

	return TRUE;
}

static gboolean
gltk_scrollable_drag_event(GltkWidget* widget, GltkEventDrag* event)
{
	if (event->longTouched)
		return FALSE;
	
	GltkScrollable* scrollable = GLTK_SCROLLABLE(widget);

	USING_PRIVATE(widget);
	GltkAllocation allocation = gltk_widget_get_allocation(widget);
	GltkAllocation childAllocation = gltk_widget_get_allocation(priv->widget);

	scrollable->offset.x = CLAMP(scrollable->offset.x + event->dx, -(childAllocation.width - allocation.width) - scrollable->paddingRight, scrollable->paddingLeft);
	scrollable->offset.y = CLAMP(scrollable->offset.y + event->dy, -(childAllocation.height - allocation.height) - scrollable->paddingBottom, scrollable->paddingTop);

	//stop if we didn't actually scroll
	if (childAllocation.x == scrollable->offset.x && childAllocation.y == scrollable->offset.y)
		return FALSE;

	childAllocation.x = scrollable->offset.x;
	childAllocation.y = scrollable->offset.y;

	//priv->widget->allocation = childAllocation;
	gltk_widget_update_allocation(priv->widget, childAllocation);

	gltk_screen_invalidate(widget->screen);

	return TRUE;
}


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

#include <glib.h>

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
static void gltk_scrollable_render(GltkWidget* widget);
static void gltk_scrollable_set_window(GltkWidget* widget, GltkWindow* window);
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
	gltkwidget_class->render = gltk_scrollable_render;
	gltkwidget_class->set_window = gltk_scrollable_set_window;
	gltkwidget_class->drag_event = gltk_scrollable_drag_event;
}

static void
gltk_scrollable_init(GltkScrollable* self)
{
	USING_PRIVATE(self);

	priv->widget = NULL;
}

static void
gltk_scrollable_dispose(GObject* gobject)
{
	GltkScrollable* self = GLTK_SCROLLABLE(gobject);
	USING_PRIVATE(self);

	if (priv->widget)
	{
		gltk_widget_unparent(priv->widget);
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
		g_object_unref(G_OBJECT(priv->widget));
	}

	g_object_ref_sink(G_OBJECT(widget));
	priv->widget = widget;
	gltk_widget_set_parent(widget, GLTK_WIDGET(scrollable));
	gltk_widget_set_window(widget, GLTK_WIDGET(scrollable)->window);
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
}

static void
gltk_scrollable_size_allocate(GltkWidget* widget, GltkAllocation* allocation)
{
	USING_PRIVATE(widget);

	if (priv->widget)
	{
		gltk_widget_size_allocate(priv->widget, *allocation);
	}

	GLTK_WIDGET_CLASS(gltk_scrollable_parent_class)->size_allocate(widget, allocation);
}

static void
gltk_scrollable_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);

	if (priv->widget)
		gltk_widget_render(priv->widget);
}

static void
gltk_scrollable_set_window(GltkWidget* widget, GltkWindow* window)
{
	USING_PRIVATE(widget);

	gltk_widget_set_window(priv->widget, window);

	GLTK_WIDGET_CLASS(gltk_scrollable_parent_class)->set_window(widget, window);

}

static gboolean
gltk_scrollable_drag_event(GltkWidget* widget, GltkEventDrag* event)
{
	g_message("scrollable should pan now");
	return TRUE; //or FALSE if we cannot scroll at all?
}


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

G_DEFINE_TYPE(GltkWindow, gltk_window, G_TYPE_OBJECT)

#define USING_PRIVATE(obj) GltkWindowPrivate* priv = GLTK_WINDOW_GET_PRIVATE(obj)
#define GLTK_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_WINDOW, GltkWindowPrivate))

static void gltk_window_dispose(GObject* gobject);
static void gltk_window_finalize(GObject* gobject);

typedef struct _GltkWindowPrivate		GltkWindowPrivate;
struct _GltkWindowPrivate
{
	int width;
	int height;

	GltkWidget* root;

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
	USING_PRIVATE(window);

	if (!priv->root)
		return;
	
	gltk_widget_render(priv->root);
}

void
gltk_window_set_root(GltkWindow* window, GltkWidget* widget)
{
	USING_PRIVATE(window);

	g_object_ref(G_OBJECT(widget));

	priv->root = widget;

	if (priv->callbacks.request_render)
		priv->callbacks.request_render();
}

GQuark
gltk_window_error_quark()
{
	return g_quark_from_static_string("gltk-window-error-quark");
}

/*********************
 * Private Functions *
 *********************/

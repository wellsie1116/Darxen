/* gltkbin.c
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

#include "gltkbin.h"

G_DEFINE_TYPE(GltkBin, gltk_bin, GLTK_TYPE_BOX)

#define USING_PRIVATE(obj) GltkBinPrivate* priv = GLTK_BIN_GET_PRIVATE(obj)
#define GLTK_BIN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_BIN, GltkBinPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkBinPrivate		GltkBinPrivate;
struct _GltkBinPrivate
{
	GltkWidget* widget;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void gltk_bin_dispose(GObject* gobject);
static void gltk_bin_finalize(GObject* gobject);

static void gltk_bin_size_request(GltkWidget* widget, GltkSize* size);
static void gltk_bin_size_allocate(GltkWidget* widget, GltkAllocation* allocation);

static void
gltk_bin_class_init(GltkBinClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkBinPrivate));
	
	gobject_class->dispose = gltk_bin_dispose;
	gobject_class->finalize = gltk_bin_finalize;

	gltkwidget_class->size_request = gltk_bin_size_request;
	gltkwidget_class->size_allocate = gltk_bin_size_allocate;
}

static void
gltk_bin_init(GltkBin* self)
{
	USING_PRIVATE(self);

	priv->widget = NULL;
}

static void
gltk_bin_dispose(GObject* gobject)
{
	GltkBin* self = GLTK_BIN(gobject);
	USING_PRIVATE(self);

	//free and release references
	if (priv->widget)
	{
		gltk_box_remove_widget(GLTK_BOX(self), priv->widget);
		g_object_unref(G_OBJECT(priv->widget));
		priv->widget = NULL;
	}

	G_OBJECT_CLASS(gltk_bin_parent_class)->dispose(gobject);
}

static void
gltk_bin_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_bin_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_bin_new(GltkWidget* widget)
{
	GObject *gobject = g_object_new(GLTK_TYPE_BIN, NULL);
	GltkBin* self = GLTK_BIN(gobject);

	USING_PRIVATE(self);
	gltk_bin_set_widget(self, widget);

	return (GltkWidget*)gobject;
}

void
gltk_bin_set_widget(GltkBin* bin, GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_BIN(bin));
	g_return_if_fail(!widget || GLTK_IS_WIDGET(widget));
	USING_PRIVATE(bin);

	if (priv->widget)
	{
		gltk_box_remove_widget(GLTK_BOX(bin), priv->widget);
		g_object_unref(G_OBJECT(priv->widget));
		priv->widget = NULL;
	}

	if (widget)
	{
		g_object_ref_sink(G_OBJECT(widget));
		priv->widget = widget;
		gltk_box_append_widget(GLTK_BOX(bin), widget, TRUE, TRUE);
	}
}

GQuark
gltk_bin_error_quark()
{
	return g_quark_from_static_string("gltk-bin-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_bin_size_request(GltkWidget* widget, GltkSize* size)
{
	USING_PRIVATE(widget);
	
	gltk_widget_size_request(priv->widget, size);
}

static void
gltk_bin_size_allocate(GltkWidget* widget, GltkAllocation* allocation)
{
	USING_PRIVATE(widget);

	if (priv->widget)
	{
		GltkAllocation childAllocation = {0, 0, allocation->width, allocation->height};
		gltk_widget_size_allocate(priv->widget, childAllocation);
	}

	GLTK_WIDGET_CLASS(gltk_bin_parent_class)->size_allocate(widget, allocation);
}


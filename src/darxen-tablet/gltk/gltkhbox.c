/* gltkhbox.c
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

#include "gltkhbox.h"

G_DEFINE_TYPE(GltkHBox, gltk_hbox, GLTK_TYPE_WIDGET)

#define USING_PRIVATE(obj) GltkHBoxPrivate* priv = GLTK_HBOX_GET_PRIVATE(obj)
#define GLTK_HBOX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_HBOX, GltkHBoxPrivate))

typedef struct
{
	GltkWidget* widget;
	gboolean expand;
	gboolean fill;
} Child;

enum
{
	LAST_SIGNAL
};

typedef struct _GltkHBoxPrivate		GltkHBoxPrivate;
struct _GltkHBoxPrivate
{
	GSList* children; //Child
};

//static guint signals[LAST_SIGNAL] = {0,};

static void gltk_hbox_dispose(GObject* gobject);
static void gltk_hbox_finalize(GObject* gobject);

void gltk_hbox_size_request(GltkWidget* widget, GltkSize* size);
void gltk_hbox_size_allocate(GltkWidget* widget, GltkSize size);

static void
gltk_hbox_class_init(GltkHBoxClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkHBoxPrivate));
	
	gobject_class->dispose = gltk_hbox_dispose;
	gobject_class->finalize = gltk_hbox_finalize;

	gltkwidget_class->size_request = gltk_hbox_size_request;
	gltkwidget_class->size_allocate = gltk_hbox_size_allocate;
}

static void
gltk_hbox_init(GltkHBox* self)
{
	USING_PRIVATE(self);

	priv->children = NULL;
}

static void
gltk_hbox_dispose(GObject* gobject)
{
	GltkHBox* self = GLTK_HBOX(gobject);
	USING_PRIVATE(self);

	if (priv->children)
	{
		GSList* pChildren = priv->children;
		while (pChildren)
		{
			Child* child = (Child*)pChildren->data;

			g_object_unref(G_OBJECT(child->widget));
			g_free(child);

			pChildren = pChildren->next;
		}
		g_slist_free(priv->children);
		priv->children = NULL;
	}

	G_OBJECT_CLASS(gltk_hbox_parent_class)->dispose(gobject);
}

static void
gltk_hbox_finalize(GObject* gobject)
{
	GltkHBox* self = GLTK_HBOX(gobject);
	USING_PRIVATE(self);

	//free memory

	G_OBJECT_CLASS(gltk_hbox_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_hbox_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_HBOX, NULL);
	GltkHBox* self = GLTK_HBOX(gobject);

	USING_PRIVATE(self);

	return (GltkHBox*)gobject;
}

void
gltk_hbox_append_widget	(GltkHBox* hbox, GltkWidget* widget, gboolean expand, gboolean fill)
{
	USING_PRIVATE(hbox);

	Child* child = g_new(Child, 1);
	child->widget = widget;
	child->expand = expand;
	child->fill = fill;

	priv->children = g_slist_append(priv->children, child);
}


GQuark
gltk_hbox_error_quark()
{
	return g_quark_from_static_string("gltk-hbox-error-quark");
}

/*********************
 * Private Functions *
 *********************/

void
gltk_hbox_size_request(GltkWidget* widget, GltkSize* size)
{
	USING_PRIVATE(widget);

	size->width = 0;
	size->height = 0;
	
	GSList* pChildren = priv->children;
	while (pChildren)
	{
		Child* child = (Child*)pChildren->data;
		GltkSize childSize;
	
		gltk_widget_size_request(child->widget, &childSize);
		size->width += childSize.width;
		size->height = MAX(size->height, childSize.height);
	
		pChildren = pChildren->next;
	}
}

void
gltk_hbox_size_allocate(GltkWidget* widget, GltkSize size)
{
	USING_PRIVATE(widget);

	int extraWidth = 0;
	int noExpandCount = 0;
	int extraChildWidth;

	//determine the amount of extra space we have
	GSList* pChildren = priv->children;
	while (pChildren)
	{
		Child* child = (Child*)pChildren->data;

		if (!child->expand)
		{
			GltkSize childSize = child->widget->requisition;
			extraWidth += childSize.width;
			noExpandCount++;
		}
	
		pChildren = pChildren->next;
	}

	//divide the space
	if (noExpandCount)
		extraChildWidth = extraWidth / noExpandCount;

	//allocate space for the children, diving the extra space appropriately
	pChildren = priv->children;
	while (pChildren)
	{
		Child* child = (Child*)pChildren->data;
		GltkSize childSize = child->widget->requisition;
		childSize.height = size.height;
	
		if (child->expand)
		{
			if (child->fill)
			{
				if (noExpandCount > 1)
				{
					childSize.width += extraChildWidth;
				}
				else
				{
					childSize.width += extraWidth;
				}
			}
			noExpandCount--;
			extraWidth -= extraChildWidth;
		}

		gltk_widget_size_allocate(child->widget, childSize);
	
		pChildren = pChildren->next;
	}
}

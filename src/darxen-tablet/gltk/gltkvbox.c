/* gltkvbox.c
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

#include "gltkvbox.h"

G_DEFINE_TYPE(GltkVBox, gltk_vbox, GLTK_TYPE_BOX)

#define USING_PRIVATE(obj) GltkVBoxPrivate* priv = GLTK_VBOX_GET_PRIVATE(obj)
#define GLTK_VBOX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_VBOX, GltkVBoxPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkVBoxPrivate		GltkVBoxPrivate;
struct _GltkVBoxPrivate
{
	int dummy;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void gltk_vbox_dispose(GObject* gobject);
static void gltk_vbox_finalize(GObject* gobject);

static void gltk_vbox_size_request(GltkWidget* widget, GltkSize* size);
static void gltk_vbox_size_allocate(GltkWidget* widget, GltkAllocation* allocation);

static void
gltk_vbox_class_init(GltkVBoxClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkVBoxPrivate));
	
	gobject_class->dispose = gltk_vbox_dispose;
	gobject_class->finalize = gltk_vbox_finalize;
	
	gltkwidget_class->size_request = gltk_vbox_size_request;
	gltkwidget_class->size_allocate = gltk_vbox_size_allocate;
}

static void
gltk_vbox_init(GltkVBox* self)
{
}

static void
gltk_vbox_dispose(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_vbox_parent_class)->dispose(gobject);
}

static void
gltk_vbox_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_vbox_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_vbox_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_VBOX, NULL);

	return (GltkWidget*)gobject;
}


GQuark
gltk_vbox_error_quark()
{
	return g_quark_from_static_string("gltk-vbox-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_vbox_size_request(GltkWidget* widget, GltkSize* size)
{
	GltkBox* box = GLTK_BOX(widget);

	size->width = 0;
	size->height = 0;
	
	GList* pChildren = box->children;
	while (pChildren)
	{
		GltkBoxChild* child = (GltkBoxChild*)pChildren->data;
		GltkSize childSize;
	
		gltk_widget_size_request(child->widget, &childSize);
		size->width = MAX(size->width, childSize.width);
		size->height += childSize.height;
	
		pChildren = pChildren->next;
	}
	GLTK_WIDGET_CLASS(gltk_vbox_parent_class)->size_request(widget, size);
}

static void
gltk_vbox_size_allocate(GltkWidget* widget, GltkAllocation* allocation)
{
	GltkBox* box = GLTK_BOX(widget);

	int y = 0;

	GltkSize requisition;
   	gltk_widget_size_request(widget, &requisition);

	int extraHeight = allocation->height - requisition.height;

	//allocate space for the children, diving the extra space appropriately
	GList* pChildren = box->children;
	while (pChildren)
	{
		GltkBoxChild* child = (GltkBoxChild*)pChildren->data;
		GltkSize childSize;
		gltk_widget_size_request(child->widget, &childSize);
		GltkAllocation childAllocation = {0, 0, allocation->width, childSize.height};
	
		if (child->expand)
		{
			int addHeight = extraHeight / box->expandCount;
			
			if (child->fill)
			{
				childAllocation.y = y;

				childAllocation.height += addHeight;

				y += childAllocation.height;
			}
			else
			{
				childAllocation.y = y + addHeight / 2;

				y += childAllocation.height + addHeight;
			}
		}
		else
		{
			childAllocation.y = y;

			y += childAllocation.height;
		}

		gltk_widget_size_allocate(child->widget, childAllocation);
	
		pChildren = pChildren->next;
	}
	GLTK_WIDGET_CLASS(gltk_vbox_parent_class)->size_allocate(widget, allocation);
}


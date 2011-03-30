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

G_DEFINE_TYPE(GltkHBox, gltk_hbox, GLTK_TYPE_BOX)

#define USING_PRIVATE(obj) GltkHBoxPrivate* priv = GLTK_HBOX_GET_PRIVATE(obj)
#define GLTK_HBOX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_HBOX, GltkHBoxPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkHBoxPrivate		GltkHBoxPrivate;
struct _GltkHBoxPrivate
{
	int dummy;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void gltk_hbox_dispose(GObject* gobject);
static void gltk_hbox_finalize(GObject* gobject);

static void gltk_hbox_size_request(GltkWidget* widget, GltkSize* size);
static void gltk_hbox_size_allocate(GltkWidget* widget, GltkAllocation* allocation);

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
}

static void
gltk_hbox_dispose(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_hbox_parent_class)->dispose(gobject);
}

static void
gltk_hbox_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_hbox_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_hbox_new(int spacing)
{
	GObject *gobject = g_object_new(GLTK_TYPE_HBOX, NULL);

	GLTK_BOX(gobject)->spacing = spacing;

	return (GltkWidget*)gobject;
}


GQuark
gltk_hbox_error_quark()
{
	return g_quark_from_static_string("gltk-hbox-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_hbox_size_request(GltkWidget* widget, GltkSize* size)
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
		size->width += childSize.width;
		size->height = MAX(size->height, childSize.height);
	
		pChildren = pChildren->next;
	}

	size->height += box->spacing * 2;
	size->width += (box->childrenCount+1) * box->spacing;

	GLTK_WIDGET_CLASS(gltk_hbox_parent_class)->size_request(widget, size);
}

static void
gltk_hbox_size_allocate(GltkWidget* widget, GltkAllocation* allocation)
{
	GltkBox* box = GLTK_BOX(widget);

	int x = box->spacing;

	GltkSize requisition;
   	gltk_widget_size_request(widget, &requisition);

	int extraWidth = allocation->width - requisition.width;// - (box->childrenCount+1)*box->spacing;

	//allocate space for the children, dividing the extra space appropriately
	GList* pChildren = box->children;
	while (pChildren)
	{
		GltkBoxChild* child = (GltkBoxChild*)pChildren->data;
		GltkSize childSize;
		gltk_widget_size_request(child->widget, &childSize);
		GltkAllocation childAllocation = {0, box->spacing, childSize.width, allocation->height - 2*box->spacing};
	
		if (child->expand)
		{
			int addWidth = extraWidth / box->expandCount;
			
			if (child->fill)
			{
				childAllocation.x = x;

				childAllocation.width += addWidth;

				x += childAllocation.width + box->spacing;
			}
			else
			{
				childAllocation.x = x + addWidth / 2;

				x += childAllocation.width + addWidth + box->spacing;
			}
		}
		else
		{
			childAllocation.x = x;

			x += childAllocation.width + box->spacing;
		}

		gltk_widget_size_allocate(child->widget, childAllocation);
	
		pChildren = pChildren->next;
	}
	GLTK_WIDGET_CLASS(gltk_hbox_parent_class)->size_allocate(widget, allocation);
}


/* gltkbox.c
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

#include "gltkbox.h"
#include "gltkscreen.h"

#include <GL/gl.h>

G_DEFINE_TYPE(GltkBox, gltk_box, GLTK_TYPE_WIDGET)

#define USING_PRIVATE(obj) GltkBoxPrivate* priv = GLTK_BOX_GET_PRIVATE(obj)
#define GLTK_BOX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_BOX, GltkBoxPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkBoxPrivate		GltkBoxPrivate;

struct _GltkBoxPrivate
{
	int dummy;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void gltk_box_dispose(GObject* gobject);
static void gltk_box_finalize(GObject* gobject);

static void gltk_box_set_screen(GltkWidget* widget, GltkScreen* screen);
static void gltk_box_render(GltkWidget* widget);
static gboolean gltk_box_event(GltkWidget* widget, GltkEvent* event);
static GltkWidget* gltk_box_find_drop_target(GltkWidget* widget, const gchar* type, const GltkRectangle* bounds);

static void
gltk_box_class_init(GltkBoxClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkBoxPrivate));
	
	gobject_class->dispose = gltk_box_dispose;
	gobject_class->finalize = gltk_box_finalize;

	gltkwidget_class->set_screen = gltk_box_set_screen;
	gltkwidget_class->render = gltk_box_render;
	gltkwidget_class->event = gltk_box_event;
	gltkwidget_class->find_drop_target = gltk_box_find_drop_target;
}

static void
gltk_box_init(GltkBox* self)
{
	self->children = NULL;
	self->expandCount = 0;
	self->childrenCount = 0;
	self->spacing = 0;
}

static void
gltk_box_dispose(GObject* gobject)
{
	GltkBox* self = GLTK_BOX(gobject);

	if (self->children)
	{
		GList* pChildren = self->children;
		while (pChildren)
		{
			GltkBoxChild* child = (GltkBoxChild*)pChildren->data;

			gltk_widget_unparent(child->widget);
			gltk_widget_set_screen(child->widget, NULL);
			g_object_unref(G_OBJECT(child->widget));
			g_free(child);

			pChildren = pChildren->next;
		}
		g_list_free(self->children);
		self->children = NULL;
	}

	G_OBJECT_CLASS(gltk_box_parent_class)->dispose(gobject);
}

static void
gltk_box_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_box_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_box_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_BOX, NULL);

	return (GltkWidget*)gobject;
}

void
gltk_box_append_widget(GltkBox* box, GltkWidget* widget, gboolean expand, gboolean fill)
{
	GltkBoxChild* child = g_new(GltkBoxChild, 1);
	child->widget = widget;
	child->expand = expand;
	child->fill = fill;
	g_object_ref_sink(G_OBJECT(widget));

	if (expand)
		box->expandCount++;
	box->childrenCount++;

	box->children = g_list_append(box->children, child);

	gltk_widget_set_parent(widget, GLTK_WIDGET(box));
	if (GLTK_WIDGET(box)->screen)
		gltk_widget_set_screen(widget, GLTK_WIDGET(box)->screen);
}

void
gltk_box_remove_widget(GltkBox* box, GltkWidget* widget)
{
	GList* pChildren = box->children;
	while (pChildren)
	{
		GltkBoxChild* child = (GltkBoxChild*)pChildren->data;
	
		if (child->widget == widget)
		{
			GList* start = g_list_remove_link(pChildren, pChildren);
			if (pChildren == box->children)
				box->children = start;

			gltk_widget_unparent(child->widget);
			gltk_widget_set_screen(child->widget, NULL);
			g_object_unref(G_OBJECT(child->widget));
			g_free(child);
			g_list_free(pChildren);
			gltk_widget_layout(GLTK_WIDGET(box));
			return;
		}
	
		pChildren = pChildren->next;
	}

	g_return_if_reached();
}


GQuark
gltk_box_error_quark()
{
	return g_quark_from_static_string("gltk-box-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_box_set_screen(GltkWidget* widget, GltkScreen* screen)
{
	GltkBox* box = GLTK_BOX(widget);

	GList* pChildren = box->children;
	while (pChildren)
	{
		GltkBoxChild* child = (GltkBoxChild*)pChildren->data;
	
		gltk_widget_set_screen(child->widget, screen);
	
		pChildren = pChildren->next;
	}

	GLTK_WIDGET_CLASS(gltk_box_parent_class)->set_screen(widget, screen);
}

static void
gltk_box_render(GltkWidget* widget)
{
	GltkBox* box = GLTK_BOX(widget);

	GList* pChildren = box->children;
	while (pChildren)
	{
		GltkBoxChild* child = (GltkBoxChild*)pChildren->data;
	
		GltkAllocation allocation = gltk_widget_get_allocation(child->widget);
		
		glTranslated(allocation.x, allocation.y, 0);

		gltk_widget_render(child->widget);

		glTranslated(-allocation.x, -allocation.y, 0);
	
		pChildren = pChildren->next;
	}
}

static gboolean
gltk_box_event(GltkWidget* widget, GltkEvent* event)
{
	GltkBox* box = GLTK_BOX(widget);

	GltkAllocation allocation = gltk_widget_get_allocation(widget);
	gboolean returnValue = FALSE;

	switch (event->type)
	{
		case GLTK_TOUCH:
		case GLTK_MULTI_DRAG:
		case GLTK_PINCH:
		case GLTK_ROTATE:
		{
			GltkEvent* childEvent = gltk_event_clone(event);

			int x;
			int y;

			switch (event->type)
			{
				case GLTK_TOUCH:
					childEvent->touch.positions->x -= allocation.x;
					childEvent->touch.positions->y -= allocation.y;
					x = childEvent->touch.positions->x;
					y = childEvent->touch.positions->y;
					break;
				case GLTK_MULTI_DRAG:
					childEvent->multidrag.center.x -= allocation.x;
					childEvent->multidrag.center.y -= allocation.y;
					x = childEvent->multidrag.center.x;
					y = childEvent->multidrag.center.y;
					break;
				case GLTK_PINCH:
					childEvent->pinch.center.x -= allocation.x;
					childEvent->pinch.center.y -= allocation.y;
					x = childEvent->pinch.center.x;
					y = childEvent->pinch.center.y;
					break;
				case GLTK_ROTATE:
					childEvent->rotate.center.x -= allocation.x;
					childEvent->rotate.center.y -= allocation.y;
					x = childEvent->rotate.center.x;
					y = childEvent->rotate.center.y;
					break;
				default:
					g_assert_not_reached();
			}

			GList* pChildren = box->children;
			while (pChildren && !returnValue)
			{
				GltkBoxChild* child = (GltkBoxChild*)pChildren->data;

				GltkAllocation childAllocation = gltk_widget_get_allocation(child->widget);

				if (childAllocation.x < x && childAllocation.x + childAllocation.width > x &&
					childAllocation.y < y && childAllocation.y + childAllocation.height > y)
				{
					returnValue = gltk_widget_send_event(child->widget, childEvent);
				}
				pChildren = pChildren->next;
			}
			gltk_event_free(childEvent);

			if (!returnValue)
				returnValue = GLTK_WIDGET_CLASS(gltk_box_parent_class)->event(widget, event);
		} break;
		default:
			//g_warning("Unhandled event type: %i", event->type);
			returnValue = GLTK_WIDGET_CLASS(gltk_box_parent_class)->event(widget, event);
	}

	return returnValue;
}

static GltkWidget*
gltk_box_find_drop_target(GltkWidget* widget, const gchar* type, const GltkRectangle* bounds)
{
	GltkWidget* res = NULL;
	GList* pChildren = GLTK_BOX(widget)->children;
	while (pChildren)
	{
		GltkBoxChild* child = (GltkBoxChild*)pChildren->data;
		GltkAllocation childAllocation = gltk_widget_get_allocation(child->widget);

		if (gltk_rectangle_intersects(bounds, (GltkRectangle*)&childAllocation))
		{
			GltkRectangle* childBounds = gltk_rectangle_copy(bounds);
			childBounds->x -= childAllocation.x;
			childBounds->y -= childAllocation.y;
			res = gltk_widget_find_drop_target(child->widget, type, childBounds);
			gltk_rectangle_free(childBounds);
		}
		if (res)
			return res;

		pChildren = pChildren->next;
	}
	//TODO add to everyone else that forwards events (table)

	return GLTK_WIDGET_CLASS(gltk_box_parent_class)->find_drop_target(widget, type, bounds);
}


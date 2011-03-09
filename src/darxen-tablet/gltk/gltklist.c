/* gltklist.c
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

#include "gltklist.h"

#include "gltkvbox.h"
#include "gltkbin.h"
#include "gltklabel.h"

#include <stdlib.h>
#include <GL/gl.h>

G_DEFINE_TYPE(GltkList, gltk_list, GLTK_TYPE_VBOX)

#define USING_PRIVATE(obj) GltkListPrivate* priv = GLTK_LIST_GET_PRIVATE(obj)
#define GLTK_LIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_LIST, GltkListPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkListPrivate		GltkListPrivate;

struct _GltkListPrivate
{
	GList* items; //GltkListItem

	GltkListItem* drag;
};

struct _GltkListItemPrivate
{
	struct {
		int x;
		int y;
	} offset;
	gboolean removed;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void gltk_list_dispose(GObject* gobject);
static void gltk_list_finalize(GObject* gobject);

static void gltk_list_set_window(GltkWidget* widget, GltkWindow* window);
static void gltk_list_render(GltkWidget* widget);

static gboolean gltk_list_bin_touch_event(GltkWidget* widget, GltkEventTouch* event, GltkListItem* item);
static gboolean gltk_list_bin_long_touch_event(GltkWidget* widget, GltkEventClick* event, GltkListItem* item);
static gboolean gltk_list_bin_drag_event(GltkWidget* widget, GltkEventDrag* event, GltkListItem* item);

static void
gltk_list_class_init(GltkListClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkListPrivate));
	
	gobject_class->dispose = gltk_list_dispose;
	gobject_class->finalize = gltk_list_finalize;

	gltkwidget_class->set_window = gltk_list_set_window;
	gltkwidget_class->render = gltk_list_render;
}

static void
gltk_list_init(GltkList* self)
{
	USING_PRIVATE(self);

	priv->items = NULL;
	priv->drag = NULL;
}

static void
gltk_list_dispose(GObject* gobject)
{
	GltkList* self = GLTK_LIST(gobject);
	USING_PRIVATE(self);

	if (priv->items)
	{
		GList* pItems = priv->items;
		while (pItems)
		{
			GltkListItem* item = (GltkListItem*)pItems->data;
		
			g_object_unref(G_OBJECT(item->widget));
			g_free(item->priv);
			g_free(item);
		
			pItems = pItems->next;
		}
		g_list_free(priv->items);
		priv->items = NULL;
	}

	G_OBJECT_CLASS(gltk_list_parent_class)->dispose(gobject);
}

static void
gltk_list_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_list_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_list_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_LIST, NULL);

	return (GltkWidget*)gobject;
}

GltkListItem*
gltk_list_add_item(GltkList* list, GltkWidget* widget, gpointer data)
{
	g_return_val_if_fail(GLTK_IS_LIST(list), NULL);
	g_return_val_if_fail(GLTK_IS_WIDGET(widget), NULL);
	USING_PRIVATE(list);

	g_object_ref_sink(G_OBJECT(widget));
	GltkWidget* bin = gltk_bin_new(widget);
	gltk_box_append_widget(GLTK_BOX(list), bin, FALSE, FALSE);

	GltkListItem* item = g_new(GltkListItem, 1);
	item->list = list;
	item->widget = widget;
	item->data = data;
	item->priv = g_new(GltkListItemPrivate, 1);

	priv->items = g_list_append(priv->items, item);
	
	g_signal_connect(bin, "touch-event", (GCallback)gltk_list_bin_touch_event, item);
	g_signal_connect(bin, "long-touch-event", (GCallback)gltk_list_bin_long_touch_event, item);
	g_signal_connect(bin, "drag-event", (GCallback)gltk_list_bin_drag_event, item);

	gltk_widget_layout(GLTK_WIDGET(list));
	return item;
}

void
gltk_list_remove_item(GltkList* list, GltkListItem* item)
{
	g_return_if_fail(GLTK_IS_LIST(list));
	g_return_if_fail(item);
	g_return_if_fail(item->list == list);
	USING_PRIVATE(list);

	priv->items = g_list_remove(priv->items, item);

	g_object_unref(G_OBJECT(item->widget));

	g_free(item->priv);
	g_free(item);
}

GQuark
gltk_list_error_quark()
{
	return g_quark_from_static_string("gltk-list-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_list_set_window(GltkWidget* widget, GltkWindow* window)
{
	USING_PRIVATE(widget);

	GList* pItems = priv->items;
	while (pItems)
	{
		GltkListItem* item = (GltkListItem*)pItems->data;
	
		gltk_widget_set_window(item->widget, window);
	
		pItems = pItems->next;
	}

	GLTK_WIDGET_CLASS(gltk_list_parent_class)->set_window(widget, window);
}

static void
gltk_list_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);

	GLTK_WIDGET_CLASS(gltk_list_parent_class)->render(widget);

	if (priv->drag)
	{
		GList* pChildren = GLTK_BOX(widget)->children;
		GList* pItems = priv->items;
		GltkBoxChild* child = NULL;
		while (pChildren && pItems)
		{
			child = (GltkBoxChild*)pChildren->data;
			if (priv->drag == (GltkListItem*)pItems->data)
				break;
			pChildren = pChildren->next;
			pItems = pItems->next;
		}
		g_assert(child);

		static float colorHighlightDark[] = {1.0f, 0.65f, 0.16f, 0.5f};
		int offsetX = priv->drag->priv->offset.x;
		int offsetY = priv->drag->priv->offset.y;
		if (abs(offsetX) < 10)
			offsetX = 0;

		glPushMatrix();
		{
			//GltkAllocation myAllocation = gltk_widget_get_allocation(widget);
			//g_message("Allocation: %i %i", myAllocation.x, myAllocation.y);
			//glTranslatef(myAllocation.x, myAllocation.y, 0.0f);

			//overlay a rectangle on top of the widget in the list
			GltkAllocation allocation = gltk_widget_get_allocation(child->widget);
			glColor4fv(colorHighlightDark);
			glRectf(allocation.x, allocation.y, allocation.x + allocation.width, allocation.y + allocation.height);

			//draw a line between stuff
			glColor3fv(colorHighlightDark);
			glBegin(GL_LINES);
			{
				int x = allocation.x + allocation.width / 2;
				int y = allocation.y + allocation.height / 2;
				glVertex2f(x, y);
				glVertex2f(x + offsetX, y + offsetY);
			}
			glEnd();

			//draw a rectangle behind our floating widget
			glColor4fv(colorHighlightDark);
			//GltkAllocation childAllocation = gltk_widget_get_allocation(priv->drag->widget);
			glTranslatef(allocation.x+offsetX, allocation.y+offsetY, 0.0f);
			glRectf(0, 0, allocation.width, allocation.height);
			gltk_widget_render(priv->drag->widget);
		}
		glPopMatrix();
	}
}

static gboolean
gltk_list_bin_touch_event(GltkWidget* widget, GltkEventTouch* event, GltkListItem* item)
{
	USING_PRIVATE(item->list);

	switch (event->touchType)
	{
		case TOUCH_BEGIN:
			gltk_window_set_widget_pressed(widget->window, widget);
			break;
		case TOUCH_END:
			gltk_window_set_widget_unpressed(widget->window, widget);
			if (priv->drag && priv->drag == item)
			{
				priv->drag = NULL;
				gltk_window_invalidate(widget->window);
			}
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

static gboolean
gltk_list_bin_long_touch_event(GltkWidget* widget, GltkEventClick* event, GltkListItem* item)
{
	USING_PRIVATE(item->list);

	priv->drag = item;

	item->priv->offset.x = 0;
	item->priv->offset.y = 0;

	gltk_window_invalidate(widget->window);

	return TRUE;
}

static GList*
move_node_forward(GList* list, GList* node)
{
	//last element
	if (!node->next)
		return list;

	//first element
	if (list == node)
		list = node->next;

	GList* next = node->next;

	if (node->prev)
		node->prev->next = next;
	next->prev = node->prev;

	node->next = next->next;
	if (node->next)
		next->next->prev = node;

	node->prev = next;
	next->next = node;

	return list;
}

static GList*
move_node_back(GList* list, GList* node)
{
	//last element
	if (!node->prev)
		return list;

	return move_node_forward(list, node->prev);
}

static gboolean
gltk_list_bin_drag_event(GltkWidget* widget, GltkEventDrag* event, GltkListItem* item)
{
	USING_PRIVATE(item->list);

	if (!priv->drag)
		return FALSE;
	
	item->priv->offset.x += event->dx;
	item->priv->offset.y += event->dy;
	
	//find our child and item in our lists
	GList* pChildren = GLTK_BOX(item->list)->children;
	GList* pItems = priv->items;
	while (pChildren && pItems)
	{
		if (item == (GltkListItem*)pItems->data)
			break;
		pChildren = pChildren->next;
		pItems = pItems->next;
	}
	g_assert(pChildren && pItems);

	GltkAllocation allocation = gltk_widget_get_allocation(priv->drag->widget);

	if (item->priv->offset.y > allocation.height)
	{
		//move item further down in out items list
		//ditto for our vbox's widgets
		if (pItems->next)
		{
			item->priv->offset.y -= gltk_widget_get_allocation(((GltkListItem*)pItems->next->data)->widget).height;

			GLTK_BOX(item->list)->children = move_node_forward(GLTK_BOX(item->list)->children, pChildren);
			priv->items = move_node_forward(priv->items, pItems);

			gltk_window_layout(widget->window);
		}
	}
	else if (item->priv->offset.y < -allocation.height)
	{
		if (pItems->prev)
		{
			item->priv->offset.y += gltk_widget_get_allocation(((GltkListItem*)pItems->prev->data)->widget).height;

			GLTK_BOX(item->list)->children = move_node_back(GLTK_BOX(item->list)->children, pChildren);
			priv->items = move_node_back(priv->items, pItems);

			gltk_window_layout(widget->window);
		}
	}

	gltk_widget_invalidate(GLTK_WIDGET(item->list));

	//g_message("A bin in a list was dragged, nomming event");
	return TRUE;
}


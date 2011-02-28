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

#include <GL/gl.h>

G_DEFINE_TYPE(GltkList, gltk_list, GLTK_TYPE_SCROLLABLE)

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

	GltkWidget* vbox;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void gltk_list_dispose(GObject* gobject);
static void gltk_list_finalize(GObject* gobject);

static gboolean gltk_list_event(GltkWidget* widget, GltkEvent* event);
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

	gltkwidget_class->event = gltk_list_event;
	gltkwidget_class->set_window = gltk_list_set_window;
	gltkwidget_class->render = gltk_list_render;
}

static void
gltk_list_init(GltkList* self)
{
	USING_PRIVATE(self);

	priv->vbox = gltk_vbox_new();
	gltk_scrollable_set_widget(GLTK_SCROLLABLE(self), priv->vbox);

	priv->items = NULL;
	priv->drag = NULL;
}

static void
gltk_list_dispose(GObject* gobject)
{
	GltkList* self = GLTK_LIST(gobject);
	USING_PRIVATE(self);

	//TODO: free and release references

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
	gltk_box_append_widget(GLTK_BOX(priv->vbox), bin, FALSE, FALSE);

	GltkListItem* item = g_new(GltkListItem, 1);
	item->widget = widget;
	item->data = data;

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
	USING_PRIVATE(list);

	priv->items = g_list_remove(priv->items, item);

	g_object_unref(G_OBJECT(item->widget));

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


static gboolean
gltk_list_event(GltkWidget* widget, GltkEvent* event)
{
	GltkList* list = GLTK_LIST(widget);
	USING_PRIVATE(list);

	gboolean returnValue = FALSE;

	switch (event->type)
	{
		case GLTK_TOUCH:
		{
			//first, try to pass the event to an item
			returnValue = gltk_widget_send_event(priv->vbox, event);

			if (!returnValue)
			{
				//The event is ours to process, defer to our parent class to emit out specific event types
				returnValue = GLTK_WIDGET_CLASS(gltk_list_parent_class)->event(widget, event);
			}
		} break;

		default:
			//let other events pass through
			returnValue = GLTK_WIDGET_CLASS(gltk_list_parent_class)->event(widget, event);
	}

	return returnValue;
}

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
		GList* pChildren = GLTK_BOX(priv->vbox)->children;
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

		GltkAllocation allocation = gltk_widget_get_allocation(child->widget);
		GltkAllocation childAllocation = gltk_widget_get_allocation(priv->drag->widget);
		glTranslatef(allocation.x+childAllocation.x, allocation.y+childAllocation.y, 0.5f);
		gltk_widget_render(priv->drag->widget);
		glTranslatef(-allocation.x-childAllocation.x, -allocation.y-childAllocation.y, -0.5f);
	}
}

static gboolean
gltk_list_bin_touch_event(GltkWidget* widget, GltkEventTouch* event, GltkListItem* item)
{
	GltkList* list = GLTK_LIST(widget->parentWidget->parentWidget); //ugly
	USING_PRIVATE(list);

	switch (event->touchType)
	{
		case TOUCH_BEGIN:
			gltk_window_set_widget_pressed(widget->window, widget);
			break;
		case TOUCH_END:
			gltk_window_set_widget_unpressed(widget->window, widget);
			if (priv->drag)
			{
				GList* pChildren = GLTK_BOX(priv->vbox)->children;
				GList* pItems = priv->items;
				while (pChildren && pItems)
				{
					GltkBoxChild* child = (GltkBoxChild*)pChildren->data;
					if (item == (GltkListItem*)pItems->data)
					{
						GltkBin* bin = GLTK_BIN(child->widget);
						gltk_bin_set_widget(bin, item->widget);
						break;
					}

					pChildren = pChildren->next;
					pItems = pItems->next;
				}

				priv->drag = NULL;
				gltk_window_layout(widget->window);
				gltk_window_invalidate(widget->window);
			}
			break;
		default:
			g_message("bin touch event - ignored");
			return FALSE;
	}
	g_message("bin touch event - handled");
	return TRUE;
}

static gboolean
gltk_list_bin_long_touch_event(GltkWidget* widget, GltkEventClick* event, GltkListItem* item)
{
	GltkList* list = GLTK_LIST(widget->parentWidget->parentWidget); //ugly
	USING_PRIVATE(list);
	priv->drag = item;

	GltkWidget* bin = item->widget->parentWidget;
	
	GltkAllocation allocation = gltk_widget_get_allocation(priv->drag->widget);
	allocation.x = 0;
	allocation.y = 0;
	gltk_widget_size_allocate(priv->drag->widget, allocation);

	gltk_bin_set_widget(GLTK_BIN(bin), gltk_label_new("placeholder"));

	gltk_window_layout(widget->window);
	gltk_window_invalidate(widget->window);

	g_message("A bin in a list was long touched, nomming event");

	return TRUE;
}

static gboolean
gltk_list_bin_drag_event(GltkWidget* widget, GltkEventDrag* event, GltkListItem* item)
{
	GltkList* list = GLTK_LIST(widget->parentWidget->parentWidget); //ugly
	USING_PRIVATE(list);

	if (!priv->drag)
		return FALSE;
	
	GltkAllocation allocation = gltk_widget_get_allocation(priv->drag->widget);

	//allocation.x += event->dx;
	allocation.y += event->dy;

	gltk_widget_size_allocate(priv->drag->widget, allocation);

	gltk_widget_invalidate(GLTK_WIDGET(list));

	g_message("A bin in a list was dragged, nomming event");
	return TRUE;
}


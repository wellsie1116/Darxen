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

#include "gltkmarshal.h"
#include "gltkvbox.h"
#include "gltkbin.h"
#include "gltklabel.h"
#include "gltkscreen.h"

#include <stdlib.h>
#include <GL/gl.h>

G_DEFINE_TYPE(GltkList, gltk_list, GLTK_TYPE_VBOX)

#define USING_PRIVATE(obj) GltkListPrivate* priv = GLTK_LIST_GET_PRIVATE(obj)
#define GLTK_LIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_LIST, GltkListPrivate))

enum
{
	ITEM_INSERTED,
	ITEM_MOVED,
	ITEM_DELETED,
	CONVERT_DROPPED_ITEM,

	LAST_SIGNAL
};

enum
{
	PROP_0,
	PROP_DELETABLE,

	N_PROPERTIES
};

typedef struct _GltkListPrivate		GltkListPrivate;

struct _GltkListPrivate
{
	GList* items; //GltkListItem
	gboolean deletable;

	GltkListItem* drag;

	guint renderOverlayId;
};

struct _GltkListItemPrivate
{
	struct {
		int x;
		int y;
	} offset;
	gboolean removed;
	GltkWidget* bin;
};

static guint signals[LAST_SIGNAL] = {0,};
static GParamSpec* properties[N_PROPERTIES] = {0,};

gboolean gltk_screen_drop_list_item(GltkScreen* screen, GltkListItem* item);

static void gltk_list_dispose(GObject* gobject);
static void gltk_list_finalize(GObject* gobject);

static void	gltk_list_set_property	(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec);
static void	gltk_list_get_property	(GObject* object, guint property_id, GValue* value, GParamSpec* pspec);

static void gltk_list_set_screen(GltkWidget* widget, GltkScreen* screen);
static void gltk_list_render(GltkWidget* widget);
static void gltk_list_drop_item(GltkWidget* widget, const gchar* type, const gpointer data);

static gboolean gltk_list_bin_touch_event(GltkWidget* widget, GltkEventTouch* event, GltkListItem* item);
static gboolean gltk_list_bin_long_touch_event(GltkWidget* widget, GltkEventClick* event, GltkListItem* item);
static gboolean gltk_list_bin_drag_event(GltkWidget* widget, GltkEventDrag* event, GltkListItem* item);

static void
gltk_list_class_init(GltkListClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkListPrivate));
	
	signals[ITEM_INSERTED] = 
		g_signal_new(	"item-inserted",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkListClass, item_inserted),
						NULL, NULL,
						g_cclosure_user_marshal_VOID__POINTER_INT,
						G_TYPE_NONE, 2,
						G_TYPE_POINTER, G_TYPE_INT);
	
	signals[ITEM_MOVED] = 
		g_signal_new(	"item-moved",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkListClass, item_moved),
						NULL, NULL,
						g_cclosure_user_marshal_VOID__POINTER_INT,
						G_TYPE_NONE, 2,
						G_TYPE_POINTER, G_TYPE_INT);
	
	signals[ITEM_DELETED] = 
		g_signal_new(	"item-deleted",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkListClass, item_deleted),
						NULL, NULL,
						g_cclosure_marshal_VOID__POINTER,
						G_TYPE_NONE, 1,
						G_TYPE_POINTER);
	
	signals[CONVERT_DROPPED_ITEM] = 
		g_signal_new(	"convert-dropped-item",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkListClass, convert_dropped_item),
						NULL, NULL,
						g_cclosure_user_marshal_POINTER__STRING_POINTER,
						G_TYPE_POINTER, 2,
						G_TYPE_STRING, G_TYPE_POINTER);
	
	gobject_class->set_property = gltk_list_set_property;
	gobject_class->get_property = gltk_list_get_property;
	gobject_class->dispose = gltk_list_dispose;
	gobject_class->finalize = gltk_list_finalize;

	gltkwidget_class->set_screen = gltk_list_set_screen;
	gltkwidget_class->render = gltk_list_render;
	gltkwidget_class->drop_item = gltk_list_drop_item;
	
	properties[PROP_DELETABLE] = 
		g_param_spec_boolean(	"deletable", "Deletable",
								"Allow items to be removed by dragging them away from the list",
								TRUE, G_PARAM_READWRITE);

	g_object_class_install_properties(gobject_class, N_PROPERTIES, properties);
}

static void
gltk_list_init(GltkList* self)
{
	USING_PRIVATE(self);

	priv->items = NULL;
	priv->drag = NULL;
	priv->deletable = FALSE;

	priv->renderOverlayId = 0;
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
	return (GltkWidget*)g_object_new(GLTK_TYPE_LIST, NULL);
}

GltkListItem*
gltk_list_insert_item(GltkList* list, GltkWidget* widget, int index, gpointer data)
{
	g_return_val_if_fail(GLTK_IS_LIST(list), NULL);
	g_return_val_if_fail(GLTK_IS_WIDGET(widget), NULL);
	USING_PRIVATE(list);
	g_return_val_if_fail(index >= 0 && index <= g_list_length(priv->items), NULL);

	g_object_ref_sink(widget);
	GltkWidget* bin = gltk_bin_new(widget);
	g_object_ref_sink(bin);
	gltk_box_insert_widget(GLTK_BOX(list), bin, index, FALSE, FALSE);

	GltkListItem* item = g_new(GltkListItem, 1);
	item->list = list;
	item->widget = widget;
	item->data = data;
	item->priv = g_new(GltkListItemPrivate, 1);
	item->priv->bin = bin;
	item->priv->removed = FALSE;

	priv->items = g_list_insert(priv->items, item, index);
	
	g_signal_connect(bin, "touch-event", (GCallback)gltk_list_bin_touch_event, item);
	g_signal_connect(bin, "long-touch-event", (GCallback)gltk_list_bin_long_touch_event, item);
	g_signal_connect(bin, "drag-event", (GCallback)gltk_list_bin_drag_event, item);

	gltk_widget_layout(GLTK_WIDGET(list));
	return item;
}

GltkListItem*
gltk_list_add_item(GltkList* list, GltkWidget* widget, gpointer data)
{
	g_return_val_if_fail(GLTK_IS_LIST(list), NULL);
	USING_PRIVATE(list);

	int index = g_list_length(priv->items);

	return gltk_list_insert_item(list, widget, index, data);
}

void
gltk_list_remove_item(GltkList* list, GltkListItem* item)
{
	g_return_if_fail(GLTK_IS_LIST(list));
	g_return_if_fail(item);
	g_return_if_fail(item->list == list);
	USING_PRIVATE(list);

	priv->items = g_list_remove(priv->items, item);

	gltk_box_remove_widget(GLTK_BOX(list), item->priv->bin);
	g_object_unref(item->priv->bin);

	g_object_unref(item->widget);

	g_free(item->priv);
	g_free(item);
}

gint
gltk_list_get_index(GltkList* list, GltkListItem* item)
{
	g_return_val_if_fail(GLTK_IS_LIST(list), -1);
	g_return_val_if_fail(item, -1);
	g_return_val_if_fail(item->list == list, -1);
	USING_PRIVATE(list);

	return g_list_index(priv->items, item);
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
gltk_list_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec)
{
	GltkList* self = GLTK_LIST(object);
	USING_PRIVATE(self);

	switch (property_id)
	{
		case PROP_DELETABLE:
			priv->deletable = g_value_get_boolean(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void
gltk_list_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec)
{
	GltkList* self = GLTK_LIST(object);
	USING_PRIVATE(self);

	switch (property_id)
	{
		case PROP_DELETABLE:
			g_value_set_boolean(value, priv->deletable);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void
gltk_list_render_overlay(GltkScreen* screen, GltkWidget* widget)
{
	USING_PRIVATE(widget);

	if (priv->drag)
	{
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
			GltkAllocation allocation = gltk_widget_get_global_allocation(priv->drag->widget);

			if (!priv->drag->priv->removed)
			{
				//overlay a rectangle on top of the widget in the list
				glColor4fv(colorHighlightDark);
				glRectf(	allocation.x,
							allocation.y,
							allocation.x + allocation.width, 
							allocation.y + allocation.height);

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
			}

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

static void
gltk_list_set_screen(GltkWidget* widget, GltkScreen* screen)
{
	USING_PRIVATE(widget);
     
	//setup/teardown hook for render-overlay
	if (!screen && priv->renderOverlayId)
	{
		g_signal_handler_disconnect(widget->screen, priv->renderOverlayId);
		priv->renderOverlayId = 0;
	}
	else if (screen)
	{
		priv->renderOverlayId = g_signal_connect(screen, "render-overlay", (GCallback)gltk_list_render_overlay, widget);   
	}

	GLTK_WIDGET_CLASS(gltk_list_parent_class)->set_screen(widget, screen);
}

static void
gltk_list_render(GltkWidget* widget)
{
	//USING_PRIVATE(widget);

	GLTK_WIDGET_CLASS(gltk_list_parent_class)->render(widget);
}

static void
gltk_list_drop_item(GltkWidget* widget, const gchar* type, const gpointer data)
{
	USING_PRIVATE(widget);

	const GltkListItem* oldItem = (const GltkListItem*)data;

	GltkListItem* item = g_new(GltkListItem, 1);
	*item = *(const GltkListItem*)data;
	item->priv = g_new(GltkListItemPrivate, 1);
	*item->priv = *((const GltkListItem*)data)->priv;
	

	//allow for a list to override how an item is transfered
	GltkWidget* newWidget = NULL;
	g_signal_emit(G_OBJECT(widget), signals[CONVERT_DROPPED_ITEM], 0, type, item, &newWidget);
	if (newWidget)
	{
		GltkSize size;
	   	gltk_widget_size_request(newWidget, &size);
		GltkAllocation allocation = gltk_widget_get_allocation(newWidget);
		allocation.width = size.width;
		allocation.height = size.height;
		gltk_widget_size_allocate(newWidget, allocation);

		item->widget = newWidget;
	}

	//initialize list item
	g_object_ref_sink(item->widget);
	item->priv->bin = gltk_bin_new(item->widget);
	g_object_ref_sink(item->priv->bin);
	gltk_bin_set_widget(GLTK_BIN(item->priv->bin), item->widget);
	item->list = GLTK_LIST(widget);
	item->priv->removed = FALSE;
	gltk_screen_swap_widget_pressed(widget->screen, item->priv->bin);

	//add the item to our items and vbox
	gltk_box_append_widget(GLTK_BOX(widget), item->priv->bin, FALSE, FALSE);
	priv->items = g_list_append(priv->items, item);
	
	//set offset
	{
		GltkAllocation before = gltk_widget_get_global_allocation(oldItem->priv->bin);
		GltkAllocation after = gltk_widget_get_global_allocation(item->priv->bin);

		item->priv->offset.x += before.x - after.x;
		item->priv->offset.y += before.y - after.y;
	}
	
	g_signal_connect(item->priv->bin, "touch-event", (GCallback)gltk_list_bin_touch_event, item);
	g_signal_connect(item->priv->bin, "long-touch-event", (GCallback)gltk_list_bin_long_touch_event, item);
	g_signal_connect(item->priv->bin, "drag-event", (GCallback)gltk_list_bin_drag_event, item);

	//set item to the current dragging item
	priv->drag = item;

	g_signal_emit(G_OBJECT(widget), signals[ITEM_INSERTED], 0, item, g_list_length(priv->items)-1);
}

static gboolean
gltk_list_bin_touch_event(GltkWidget* widget, GltkEventTouch* event, GltkListItem* item)
{
	USING_PRIVATE(item->list);

	switch (event->touchType)
	{
		case TOUCH_BEGIN:
			gltk_screen_set_widget_pressed(widget->screen, widget);
			break;
		case TOUCH_END:
			gltk_screen_set_widget_unpressed(widget->screen, widget);
			if (priv->drag && priv->drag == item)
			{
				if (priv->drag->priv->removed)
				{
					//delete item
					gltk_bin_set_widget(GLTK_BIN(item->priv->bin), NULL);

					g_object_ref(item->list);
					g_signal_emit(G_OBJECT(item->list), signals[ITEM_DELETED], 0, item);
					g_object_unref(item->list);

					g_object_unref(item->widget);
					g_object_unref(item->priv->bin);
					g_free(item->priv);
					g_free(item);
				}
				priv->drag = NULL;
				gltk_screen_invalidate(widget->screen);
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

	gltk_screen_invalidate(widget->screen);

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
	
	GltkAllocation allocation = gltk_widget_get_allocation(priv->drag->widget);

	//find our child and item in our lists
	int i = 0;
	GList* pChildren = GLTK_BOX(item->list)->children;
	GList* pItems = priv->items;
	while (pChildren && pItems)
	{
		if (item == (GltkListItem*)pItems->data)
			break;
		pChildren = pChildren->next;
		pItems = pItems->next;
		i++;
	}

	if (item->priv->removed || abs(item->priv->offset.x) > allocation.width)
	{
		g_assert((pChildren && pItems) || (!pChildren && !pItems));

		if (priv->deletable && !item->priv->removed)
		{
			//begin deletion (or maybe drop)
			priv->items = g_list_remove(priv->items, item);
			gltk_box_remove_widget(GLTK_BOX(item->list), item->priv->bin);
			gltk_widget_set_parent(item->priv->bin, GLTK_WIDGET(item->list));
			gltk_widget_set_screen(item->priv->bin, GLTK_WIDGET(item->list)->screen);
			item->priv->removed = TRUE;
		}
		else
		{
			//continue drag
			GltkWidget* target = NULL;
			gchar* targetType;
			g_object_get(item->list, "target-type", &targetType, NULL);
			if (targetType)
			{
				GltkAllocation globalAllocation = gltk_widget_get_global_allocation(priv->drag->widget);
				GltkRectangle* bounds = gltk_rectangle_new(	globalAllocation.x + item->priv->offset.x,
															globalAllocation.y + item->priv->offset.y,
															globalAllocation.width, globalAllocation.height);
				target = gltk_screen_find_drop_target(widget->screen, targetType, bounds);
				gltk_rectangle_free(bounds);
				g_free(targetType);
			}

			if (target == GLTK_WIDGET(item->list))
			{
				g_debug("An item has returned to us");
				//brought item back onto our list
				GltkAllocation allocBefore = gltk_widget_get_global_allocation(item->priv->bin);
				gltk_widget_unparent(item->priv->bin);
				gltk_widget_set_screen(item->priv->bin, NULL);

				//TODO: readd in the correct location (not two append calls)
				priv->items = g_list_append(priv->items, item);
				gltk_box_append_widget(GLTK_BOX(item->list), item->priv->bin, FALSE, FALSE);
				gltk_widget_layout(GLTK_WIDGET(item->list));
				GltkAllocation allocAfter = gltk_widget_get_global_allocation(item->priv->bin);
				item->priv->offset.x -= allocAfter.x - allocBefore.x;
				item->priv->offset.y -= allocAfter.y - allocBefore.y;
				item->priv->removed = FALSE;

			}
			else if (target)
			{
				//remove if it hadn't been removed yet
				if (!priv->deletable)
				{
					priv->items = g_list_remove(priv->items, item);
					gltk_box_remove_widget(GLTK_BOX(item->list), item->priv->bin);
					gltk_widget_set_parent(item->priv->bin, GLTK_WIDGET(item->list));
					gltk_widget_set_screen(item->priv->bin, GLTK_WIDGET(item->list)->screen);
					item->priv->removed = TRUE;
				}
				gltk_bin_set_widget(GLTK_BIN(item->priv->bin), NULL);
				//drop on another widget
				//g_critical("TODO drop list item");
				g_object_get(item->list, "target-type", &targetType, NULL);
				gltk_widget_drop_item(target, targetType, item);
				g_free(targetType);
				GltkList* list = item->list;
				g_object_unref(item->priv->bin);
				g_object_unref(item->widget);
				g_free(item->priv);
				g_free(item);
				priv->drag = NULL;
				gltk_widget_invalidate(GLTK_WIDGET(list));
				return TRUE;
			}
		}
	}
	else
	{
	
		g_assert(pChildren && pItems);

		if (item->priv->offset.y > allocation.height)
		{
			//move item further down in out items list
			//ditto for our vbox's widgets
			if (pItems->next)
			{
				item->priv->offset.y -= gltk_widget_get_allocation(((GltkListItem*)pItems->next->data)->widget).height;

				GLTK_BOX(item->list)->children = move_node_forward(GLTK_BOX(item->list)->children, pChildren);
				priv->items = move_node_forward(priv->items, pItems);

				g_signal_emit(G_OBJECT(item->list), signals[ITEM_MOVED], 0, item, i);

				gltk_screen_layout(widget->screen);
			}
		}
		else if (item->priv->offset.y < -allocation.height)
		{
			if (pItems->prev)
			{
				item->priv->offset.y += gltk_widget_get_allocation(((GltkListItem*)pItems->prev->data)->widget).height;

				GLTK_BOX(item->list)->children = move_node_back(GLTK_BOX(item->list)->children, pChildren);
				priv->items = move_node_back(priv->items, pItems);

				g_signal_emit(G_OBJECT(item->list), signals[ITEM_MOVED], 0, item, i);

				gltk_screen_layout(widget->screen);
			}
		}
	}

	gltk_widget_invalidate(GLTK_WIDGET(item->list));

	//g_message("A bin in a list was dragged, nomming event");
	return TRUE;
}


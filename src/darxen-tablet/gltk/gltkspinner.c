/* gltkspinner.c
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

#include "gltkspinner.h"

#include "gltkscreen.h"
#include "gltkvbox.h"
#include "gltklabel.h"

#include <math.h>
#include <glib.h>
#include <GL/gl.h>
#include <GL/glu.h>

G_DEFINE_TYPE(GltkSpinner, gltk_spinner, GLTK_TYPE_SCROLLABLE)

#define USING_PRIVATE(obj) GltkSpinnerPrivate* priv = GLTK_SPINNER_GET_PRIVATE(obj)
#define GLTK_SPINNER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_SPINNER, GltkSpinnerPrivate))

enum
{
	ITEM_SELECTED,

	LAST_SIGNAL
};

typedef struct _SpinnerItem				SpinnerItem;
typedef struct _GltkSpinnerPrivate		GltkSpinnerPrivate;

struct _SpinnerItem
{
	gchar* text;
	GltkWidget* label;
};

struct _GltkSpinnerPrivate
{
	GltkWidget* vbox;

	int itemHeight;

	GList* items; //SpinnerItem
	GList* pItems; //SpinnerItem
};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_spinner_dispose(GObject* gobject);
static void gltk_spinner_finalize(GObject* gobject);

static gboolean	gltk_spinner_touch_event	(GltkWidget* widget, GltkEventTouch* event);
static gboolean	gltk_spinner_drag_event		(GltkWidget* widget, GltkEventDrag* event);
static void gltk_spinner_size_allocate		(GltkWidget* widget, GltkAllocation* allocation);
static void gltk_spinner_size_request		(GltkWidget* widget, GltkSize* size);
static void gltk_spinner_render				(GltkWidget* widget);

static void
gltk_spinner_class_init(GltkSpinnerClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkSpinnerPrivate));
	
	signals[ITEM_SELECTED] = 
		g_signal_new(	"item-selected",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkSpinnerClass, item_selected),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	
	gobject_class->dispose = gltk_spinner_dispose;
	gobject_class->finalize = gltk_spinner_finalize;

	gltkwidget_class->touch_event = gltk_spinner_touch_event;
	gltkwidget_class->drag_event = gltk_spinner_drag_event;
	gltkwidget_class->size_allocate = gltk_spinner_size_allocate;
	gltkwidget_class->size_request = gltk_spinner_size_request;
	gltkwidget_class->render = gltk_spinner_render;

	klass->item_selected = NULL;
}

static void
gltk_spinner_init(GltkSpinner* self)
{
	USING_PRIVATE(self);
	
	priv->vbox = NULL;

	priv->itemHeight = 20;

	priv->items = NULL;
	priv->pItems = NULL;
}

static void
gltk_spinner_dispose(GObject* gobject)
{
	GltkSpinner* self = GLTK_SPINNER(gobject);
	USING_PRIVATE(self);

	if (priv->vbox)
	{
		g_object_unref(priv->vbox);
		priv->vbox = NULL;
	}

	if (priv->items)
	{
		GList* pItems = priv->items;
		while (pItems)
		{
			SpinnerItem* item = (SpinnerItem*)pItems->data;
		
			g_free(item->text);
			g_object_unref(item->label);

			g_slice_free(SpinnerItem, item);
		
			pItems = pItems->next;
		}

		g_list_free(priv->items);

		priv->items = NULL;
	}

	G_OBJECT_CLASS(gltk_spinner_parent_class)->dispose(gobject);
}

static void
gltk_spinner_finalize(GObject* gobject)
{
//	GltkSpinner* self = GLTK_SPINNER(gobject);
//	USING_PRIVATE(self);
//
	G_OBJECT_CLASS(gltk_spinner_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_spinner_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_SPINNER, NULL);
	GltkSpinner* self = GLTK_SPINNER(gobject);

	USING_PRIVATE(self);

	priv->vbox = gltk_vbox_new(0);

	gltk_scrollable_set_widget(GLTK_SCROLLABLE(self), priv->vbox);

	return (GltkWidget*)gobject;
}

void
gltk_spinner_add_item(GltkSpinner* spinner, const gchar* label)
{
	USING_PRIVATE(spinner);

	SpinnerItem* item  = g_slice_new(SpinnerItem);

	item->text = g_strdup(label);
	item->label = gltk_label_new(label);
	GLTK_LABEL(item->label)->color.r = 0.0f;
	GLTK_LABEL(item->label)->color.g = 0.0f;
	GLTK_LABEL(item->label)->color.b = 0.0f;
	gltk_box_append_widget(GLTK_BOX(priv->vbox), item->label, FALSE, FALSE);

	priv->items = g_list_append(priv->items, item);

	if (!priv->pItems)
		priv->pItems = priv->items;
	
	GltkSize size;
	gltk_widget_size_request(item->label, &size);

	if (size.height > priv->itemHeight)
	{
		priv->itemHeight = size.height;
		GLTK_SCROLLABLE(spinner)->paddingTop = 2*priv->itemHeight;
		GLTK_SCROLLABLE(spinner)->paddingBottom = 2*priv->itemHeight;
		size.width = -1;

		GList* pItems = priv->items;
		while (pItems)
		{
			SpinnerItem* pItem = (SpinnerItem*)pItems->data;
			gltk_widget_set_size_request(pItem->label, size);
			pItems = pItems->next;
		}

		gltk_widget_layout(GLTK_WIDGET(spinner));
	}
}

const gchar*
gltk_spinner_get_selected_item(GltkSpinner* spinner)
{
	USING_PRIVATE(spinner);

	if (!priv->pItems)
		return NULL;

	SpinnerItem* item = (SpinnerItem*)priv->pItems->data;
	return item->text;
}


GQuark
gltk_spinner_error_quark()
{
	return g_quark_from_static_string("gltk-spinner-error-quark");
}

/*********************
 * Private Functions *
 *********************/

#define BORDER_WIDTH 10
#define BORDER_HEIGHT 10

static void
gltk_spinner_size_request(GltkWidget* widget, GltkSize* size)
{
	USING_PRIVATE(widget);
	
	int width = 50;

	GList* pItems = priv->items;
	while (pItems)
	{
		SpinnerItem* item = (SpinnerItem*)pItems->data;
	
		gltk_widget_size_request(item->label, size);
		if (size->width > width)
			width = size->width;
	
		pItems = pItems->next;
	}

	size->width = width + 2*BORDER_WIDTH;
	size->height = priv->itemHeight * 5 + 2*BORDER_HEIGHT;

	widget->requisition = *size;
}

static gboolean
gltk_spinner_touch_event(GltkWidget* widget, GltkEventTouch* event)
{
	USING_PRIVATE(widget);

	gboolean res = GLTK_WIDGET_CLASS(gltk_spinner_parent_class)->touch_event(widget, event);

	if (event->touchType == TOUCH_END)
	{
		int index = round((float)GLTK_SCROLLABLE(widget)->offset.y / priv->itemHeight);
		GLTK_SCROLLABLE(widget)->offset.y = index * priv->itemHeight;
		index = -index + 2;

		GList* pItems = g_list_nth(priv->items, index);
		g_assert(pItems);
		if (pItems != priv->pItems)
		{
			priv->pItems = pItems;
			g_object_ref(widget);
			g_signal_emit(widget, signals[ITEM_SELECTED], 0);
			g_object_unref(widget);
		}
	}

	return res;

}

static gboolean
gltk_spinner_drag_event(GltkWidget* widget, GltkEventDrag* event)
{
	return GLTK_WIDGET_CLASS(gltk_spinner_parent_class)->drag_event(widget, event);
}

static void
gltk_spinner_size_allocate(GltkWidget* widget, GltkAllocation* allocation)
{
	//reserve our border
	GltkAllocation childAllocation = *allocation;
	childAllocation.x += BORDER_WIDTH;
	childAllocation.y += BORDER_HEIGHT;
	childAllocation.width -= 2*BORDER_WIDTH;
	childAllocation.height -= 2*BORDER_HEIGHT;

	GLTK_WIDGET_CLASS(gltk_spinner_parent_class)->size_allocate(widget, &childAllocation);
}

static void
gltk_spinner_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);

	GltkAllocation allocation = gltk_widget_get_allocation(widget);

	glPushMatrix();
	{
		glBegin(GL_QUADS);
		{
			//border
			glColor3f(0.65f, 0.65f, 0.71f);
			glVertex2i(allocation.width + BORDER_WIDTH, -BORDER_HEIGHT);
			glVertex2i(-BORDER_WIDTH, -BORDER_HEIGHT);
			glColor3f(0.49f, 0.49f, 0.54f);
			glVertex2i(-BORDER_WIDTH, allocation.height*0.5);
			glVertex2i(allocation.width + BORDER_WIDTH, allocation.height*0.5);

			glColor3f(0.33f, 0.33f, 0.38f);
			glVertex2i(allocation.width + BORDER_WIDTH, allocation.height*0.5);
			glVertex2i(-BORDER_WIDTH, allocation.height*0.5);
			glColor3f(0.12f, 0.12f, 0.16f);
			glVertex2i(-BORDER_WIDTH, allocation.height + BORDER_HEIGHT);
			glVertex2i(allocation.width + BORDER_WIDTH, allocation.height + BORDER_HEIGHT);

			//highlighting
			static const float heights = 1.5f;
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex2i(allocation.width, 0);
			glVertex2i(0, 0);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex2i(0, priv->itemHeight*heights);
			glVertex2i(allocation.width, priv->itemHeight*heights);
			
			glVertex2i(0, priv->itemHeight*heights);
			glVertex2i(allocation.width, priv->itemHeight*heights);
			glVertex2i(allocation.width, allocation.height - priv->itemHeight*heights);
			glVertex2i(0, allocation.height - priv->itemHeight*heights);

			glVertex2i(allocation.width, allocation.height - priv->itemHeight*heights);
			glVertex2i(0, allocation.height - priv->itemHeight*heights);
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex2i(0, allocation.height);
			glVertex2i(allocation.width, allocation.height);
		}
		glEnd();

		glTranslatef(BORDER_WIDTH, BORDER_HEIGHT, 0.0f);
		GLTK_WIDGET_CLASS(gltk_spinner_parent_class)->render(widget);
		glTranslatef(-BORDER_WIDTH, -BORDER_HEIGHT, 0.0f);

		glBegin(GL_QUADS);
		{
			//selector
			glColor4f(0.53f, 0.54f, 0.77f, 0.3f);
			glVertex2f(allocation.width, 2*priv->itemHeight);
			glVertex2f(0, 2*priv->itemHeight);
			glVertex2f(0, 2.5*priv->itemHeight);
			glVertex2f(allocation.width, 2.5*priv->itemHeight);
			
			glColor4f(0.17f, 0.17f, 0.53f, 0.3f);
			glVertex2f(allocation.width, 2.5*priv->itemHeight);
			glVertex2f(0, 2.5*priv->itemHeight);
			glVertex2i(0, 3*priv->itemHeight);
			glVertex2i(allocation.width, 3*priv->itemHeight);
		}
		glEnd();

		glBegin(GL_LINE_LOOP);
		{
			//border around selector
			glColor4f(0.54f, 0.55f, 0.63f, 0.3f);
			glVertex2f(allocation.width, 2*priv->itemHeight);
			glVertex2f(0, 2*priv->itemHeight);
			glVertex2i(0, 3*priv->itemHeight);
			glVertex2i(allocation.width, 3*priv->itemHeight);
		}
		glEnd();

	}
	glPopMatrix();


}



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
#include "gltkscrollable.h"
#include "gltkvbox.h"
#include "gltkhbox.h"
#include "gltklabel.h"

#include <math.h>
#include <glib.h>
#include <GL/gl.h>
#include <GL/glu.h>

G_DEFINE_TYPE(GltkSpinner, gltk_spinner, GLTK_TYPE_BIN)

#define USING_PRIVATE(obj) GltkSpinnerPrivate* priv = GLTK_SPINNER_GET_PRIVATE(obj)
#define GLTK_SPINNER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_SPINNER, GltkSpinnerPrivate))

#define BORDER_WIDTH 10
#define BORDER_HEIGHT 10

enum
{
	ITEM_SELECTED,

	LAST_SIGNAL
};

typedef struct _Wheel					Wheel;
typedef struct _GltkSpinnerPrivate		GltkSpinnerPrivate;

struct _Wheel
{
	GList* items; //GltkSpinnerModelItem
	GltkWidget* scrollable;
	GltkWidget* vbox;
	int index;
};

struct _GltkSpinnerPrivate
{
	GltkSpinnerModel* model;
	GltkWidget* hbox;
	Wheel* wheels;

	gboolean heightChanged;
	int itemHeight;
};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_spinner_dispose(GObject* gobject);
static void gltk_spinner_finalize(GObject* gobject);

static void		load_items						(GltkSpinner* spinner, int level, GList* items);
static gint 	find_spinner_item				(GltkSpinnerModelItem* i1, const gchar* id);
static gboolean	scrollable_touch_event			(GltkWidget* scrollable, GltkEventTouch* event, GltkSpinner* spinner);
static void		gltk_spinner_size_allocate		(GltkWidget* widget, GltkAllocation* allocation);
static void		gltk_spinner_size_request		(GltkWidget* widget, GltkSize* size);
static gboolean	gltk_spinner_drag_event			(GltkWidget* widget, GltkEventDrag* event);
static void		gltk_spinner_render				(GltkWidget* widget);

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

	gltkwidget_class->size_allocate = gltk_spinner_size_allocate;
	gltkwidget_class->size_request = gltk_spinner_size_request;
	gltkwidget_class->drag_event = gltk_spinner_drag_event;
	gltkwidget_class->render = gltk_spinner_render;

	klass->item_selected = NULL;
}

static void
gltk_spinner_init(GltkSpinner* self)
{
	USING_PRIVATE(self);
	
	priv->model = NULL;
	priv->hbox = NULL;
	priv->wheels = NULL;

	priv->heightChanged = TRUE;
	priv->itemHeight = 20;
}

static void
gltk_spinner_dispose(GObject* gobject)
{
	GltkSpinner* self = GLTK_SPINNER(gobject);
	USING_PRIVATE(self);

	if (priv->wheels)
	{
		int i;
		for (i = 0; i < priv->model->levels; i++)
		{
			g_object_unref(priv->wheels[i].vbox);
			g_object_unref(priv->wheels[i].scrollable);
			gltk_spinner_model_free_items(priv->model, priv->wheels[i].items);
		}

		g_free(priv->wheels);
		priv->wheels = NULL;
	}
	
	if (priv->hbox)
	{
		g_object_unref(priv->hbox);
		priv->hbox = NULL;
	}

	if (priv->model)
	{
		g_object_unref(priv->model);
		priv->model = NULL;
	}

	G_OBJECT_CLASS(gltk_spinner_parent_class)->dispose(gobject);
}

static void
gltk_spinner_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_spinner_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_spinner_new(GltkSpinnerModel* model)
{
	GObject *gobject = g_object_new(GLTK_TYPE_SPINNER, NULL);
	GltkSpinner* self = GLTK_SPINNER(gobject);

	USING_PRIVATE(self);

	GLTK_BIN(self)->border.width = BORDER_WIDTH;
	GLTK_BIN(self)->border.height = BORDER_HEIGHT;

	priv->hbox = gltk_hbox_new(1);

	g_object_ref(model);
	priv->model = model;

	//initialize the wheels
	priv->wheels = g_new(Wheel, priv->model->levels);
	int i;
	for (i = 0; i < priv->model->levels; i++)
	{
		Wheel* wheel = priv->wheels + i;

		wheel->vbox = gltk_vbox_new(0);
		wheel->scrollable = gltk_scrollable_new();
		g_object_ref(wheel->vbox);
		g_object_ref(wheel->scrollable);
		gltk_scrollable_set_widget(GLTK_SCROLLABLE(wheel->scrollable), wheel->vbox);
		gltk_box_append_widget(GLTK_BOX(priv->hbox), wheel->scrollable, TRUE, TRUE);

		g_signal_connect(wheel->scrollable, "touch-event", (GCallback)scrollable_touch_event, self);
		
		wheel->items = NULL;
		wheel->index = 0;
	}

	//load our toplevel items from the model 
	load_items(self, 0, gltk_spinner_model_clone_items(priv->model, priv->model->toplevel)); 

	gltk_bin_set_widget(GLTK_BIN(self), priv->hbox);

	return (GltkWidget*)gobject;
}

static int
get_selected_index(GltkSpinner* spinner, int level)
{
	USING_PRIVATE(spinner);

	return -round((float)GLTK_SCROLLABLE(priv->wheels[level].scrollable)->offset.y / priv->itemHeight) + 2;
}

static void
set_selected_index(GltkSpinner* spinner, int level, int index)
{
	USING_PRIVATE(spinner);
	
	GLTK_SCROLLABLE(priv->wheels[level].scrollable)->offset.y = (-index + 2) * priv->itemHeight;
}

const gchar*
gltk_spinner_get_selected_item(GltkSpinner* spinner, int level)
{
	g_return_val_if_fail(GLTK_IS_SPINNER(spinner), NULL);
	USING_PRIVATE(spinner);
	g_return_val_if_fail(level >= 0 && level < priv->model->levels, NULL);

	GltkSpinnerModelItem* item;
	item = (GltkSpinnerModelItem*)g_list_nth_data(priv->wheels[level].items, priv->wheels[level].index);
	g_assert(item);

	return item->id;
}

void
gltk_spinner_set_selected_index(GltkSpinner* spinner, int level, int index)
{
	g_return_if_fail(GLTK_IS_SPINNER(spinner));
	USING_PRIVATE(spinner);
	g_return_if_fail(level >= 0 && level < priv->model->levels);
	g_return_if_fail(index >= 0 && index < g_list_length(priv->wheels[level].items));

	set_selected_index(spinner, level, index);
	priv->wheels[level].index = index;
	
	if (level == priv->model->levels - 1)
	{
		g_object_ref(spinner);
		g_signal_emit(spinner, signals[ITEM_SELECTED], 0);
		g_object_unref(spinner);
	}
	else
	{
		load_items(spinner, level+1, gltk_spinner_model_get_items(priv->model, level, index));
	}
}

void
gltk_spinner_set_selected_item(GltkSpinner* spinner, int level, const gchar* id)
{
	g_return_if_fail(GLTK_IS_SPINNER(spinner));
	USING_PRIVATE(spinner);
	g_return_if_fail(level >= 0 && level < priv->model->levels);

	GList* pItems = g_list_find_custom(priv->wheels[level].items, id, (GCompareFunc)find_spinner_item);
	g_return_if_fail(pItems);

	int index = g_list_position(priv->wheels[level].items, pItems);

	gltk_spinner_set_selected_index(spinner, level, index);
}

GQuark
gltk_spinner_error_quark()
{
	return g_quark_from_static_string("gltk-spinner-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
load_items(GltkSpinner* spinner, int level, GList* items)
{
	USING_PRIVATE(spinner);

	Wheel* wheel = priv->wheels + level;

	//clean up an old mess
	if (wheel->items)
	{
		gltk_spinner_model_free_items(priv->model, wheel->items);

		g_object_unref(wheel->vbox);
		wheel->vbox = gltk_vbox_new(0);
		g_object_ref(wheel->vbox);
		gltk_scrollable_set_widget(GLTK_SCROLLABLE(wheel->scrollable), wheel->vbox);
	}

	//make a new one
	wheel->items = items;

	int itemHeight = priv->itemHeight;

	GList* pItems = wheel->items;
	while (pItems)
	{
		GltkSpinnerModelItem* item = (GltkSpinnerModelItem*)pItems->data;
	
		GltkWidget* label = gltk_label_new(item->text);
		GLTK_LABEL(label)->color.r = 0.0f;
		GLTK_LABEL(label)->color.g = 0.0f;
		GLTK_LABEL(label)->color.b = 0.0f;
		gltk_box_append_widget(GLTK_BOX(wheel->vbox), label, FALSE, FALSE);
	
		GltkSize size;
		gltk_widget_size_request(label, &size);
		if (size.height > itemHeight)
			itemHeight = size.height;

		size.width = -1;
		size.height = priv->itemHeight;
		gltk_widget_set_size_request(label, size);

		pItems = pItems->next;
	}

	//if our max height changed, relayout and resize
	if (itemHeight > priv->itemHeight)
	{
		priv->itemHeight = itemHeight;

		priv->heightChanged = TRUE;
		gltk_widget_layout(GLTK_WIDGET(spinner));
	}
	
	//select a specific item
	int len = g_list_length(wheel->items);
	if (wheel->index >= len - 1)
		wheel->index = len-1;
	if (wheel->index < 0)
		wheel->index = 0;
	set_selected_index(spinner, level, wheel->index);
		
	//set allowable amount to scroll past the vbox
	GLTK_SCROLLABLE(wheel->scrollable)->paddingTop = 2*priv->itemHeight;
	GLTK_SCROLLABLE(wheel->scrollable)->paddingBottom = ((len > 5) ? 2 : len-3)*priv->itemHeight;

	//either signal that an item was selected or recurse until we do
	if (level == priv->model->levels-1)
	{
		g_object_ref(spinner);
		g_signal_emit(spinner, signals[ITEM_SELECTED], 0);
		g_object_unref(spinner);
	}
	else
	{
		load_items(spinner, level+1, gltk_spinner_model_get_items(priv->model, level, wheel->index));
	}
}

static gint
find_spinner_item(GltkSpinnerModelItem* i1, const gchar* id)
{
	return g_strcmp0(i1->id, id);
}

static void
gltk_spinner_size_request(GltkWidget* widget, GltkSize* size)
{
	USING_PRIVATE(widget);
	
	//set a uniform height (if not already set)
	if (priv->heightChanged)
	{
		int i;
		for (i = 0; i < priv->model->levels; i++)
		{
			GList* pChildren = GLTK_BOX(priv->wheels[i].vbox)->children;
			while (pChildren)
			{
				GltkBoxChild* child = (GltkBoxChild*)pChildren->data;
			
				size->width = -1;
				size->height = priv->itemHeight;
				gltk_widget_set_size_request(child->widget, *size);

				pChildren = pChildren->next;
			}
		}
		priv->heightChanged = FALSE;
	}

	GltkSize sizeRequest = {-1, priv->itemHeight*5};
	gltk_widget_set_size_request(priv->hbox, sizeRequest);

	//get the request from the hbox
	GLTK_WIDGET_CLASS(gltk_spinner_parent_class)->size_request(widget, size);

	//persist
	widget->requisition = *size;
}

static gboolean
scrollable_touch_event(GltkWidget* scrollable, GltkEventTouch* event, GltkSpinner* spinner)
{
	USING_PRIVATE(spinner);

	if (event->touchType == TOUCH_END)
	{
		int level;
		for (level = 0; level < priv->model->levels; level++)
		{
			if (priv->wheels[level].scrollable != scrollable)
				continue;

			int index = get_selected_index(spinner, level);
			GLTK_SCROLLABLE(priv->wheels[level].scrollable)->offset.y = (-index + 2) * priv->itemHeight;

			if (index != priv->wheels[level].index)
			{
				priv->wheels[level].index = index;
				if (level == priv->model->levels - 1)
				{
					g_object_ref(spinner);
					g_signal_emit(spinner, signals[ITEM_SELECTED], 0);
					g_object_unref(spinner);
				}
				else
				{
					//load our new items into the next spinner
					load_items(spinner, level+1, gltk_spinner_model_get_items(priv->model, level, index));
					gltk_widget_layout(GLTK_WIDGET(spinner));
				}
			}
			break;
		}
	}

	return FALSE;

}

static void
gltk_spinner_size_allocate(GltkWidget* widget, GltkAllocation* allocation)
{
	//pass through allocations
	GLTK_WIDGET_CLASS(gltk_spinner_parent_class)->size_allocate(widget, allocation);
}

static gboolean
gltk_spinner_drag_event(GltkWidget* widget, GltkEventDrag* event)
{
	//om nom nom
	return TRUE;
}

static void
gltk_spinner_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);

	GltkAllocation allocation = gltk_widget_get_allocation(widget);
	allocation.width -= 2*BORDER_WIDTH;
	allocation.height -= 2*BORDER_HEIGHT;

	glPushMatrix();
	{
		glTranslated(BORDER_WIDTH, BORDER_HEIGHT, 0);
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

		//dividers
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		{
			glVertex2i(0, 0);
			glVertex2i(0, allocation.height);
			glVertex2i(allocation.width, 0);
			glVertex2i(allocation.width, allocation.height);
			int i;
			int x = 1;
			for (i = 0; i < priv->model->levels-1; i++)
			{
				Wheel* wheel = priv->wheels + i;

				GltkAllocation wheelAllocation = gltk_widget_get_allocation(wheel->scrollable);

				x += wheelAllocation.width + 1;
				glVertex2i(x, 0);
				glVertex2i(x, allocation.height);
			}
		}
		glEnd();

		GLTK_WIDGET_CLASS(gltk_spinner_parent_class)->render(widget);

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



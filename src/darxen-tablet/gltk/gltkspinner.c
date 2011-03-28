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

static void gltk_spinner_size_request	(GltkWidget* widget, GltkSize* size);
static void gltk_spinner_render			(GltkWidget* widget);

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

	priv->vbox = gltk_vbox_new(1);

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
	gltk_box_append_widget(GLTK_BOX(priv->vbox), item->label, FALSE, FALSE);

	priv->items = g_list_append(priv->items, item);

	if (!priv->pItems)
		priv->pItems = priv->items;
	
	GltkSize size;
	gltk_widget_size_request(item->label, &size);

	if (size.height > priv->itemHeight)
	{
		priv->itemHeight = size.height;
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

	size->width = width;
	size->height = priv->itemHeight * 5;

	widget->sizeRequest = *size;
}

static void
gltk_spinner_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);

	GltkAllocation allocation = gltk_widget_get_global_allocation(widget);
	GltkSize size = gltk_screen_get_window_size(widget->screen);

	//setup our rendering window how we like it
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	int offsetX = allocation.x;
	int offsetY = size.height - allocation.height - allocation.y;
	glViewport(offsetX, offsetY, allocation.width, allocation.height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, allocation.width, allocation.height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	{
		glLoadIdentity();

		glBegin(GL_QUADS);
		{
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex2i(allocation.width, 0);
			glVertex2i(0, 0);
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex2i(0, priv->itemHeight);
			glVertex2i(allocation.width, priv->itemHeight);
			
			glVertex2i(allocation.width, allocation.height - priv->itemHeight);
			glVertex2i(0, allocation.height - priv->itemHeight);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex2i(0, allocation.height);
			glVertex2i(allocation.width, allocation.height);
		}
		glEnd();

		GLTK_WIDGET_CLASS(gltk_spinner_parent_class)->render(widget);

		glBegin(GL_QUADS);
		{
			glColor4f(0.0f, 0.0f, 1.0f, 0.3f);
			glVertex2i(allocation.width, 2*priv->itemHeight);
			glVertex2i(0, 2*priv->itemHeight);
			glVertex2i(0, 3*priv->itemHeight);
			glVertex2i(allocation.width, 3*priv->itemHeight);
		}
		glEnd();

		
	
	}
	glPopMatrix();

	//undo our changes to the state
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

}



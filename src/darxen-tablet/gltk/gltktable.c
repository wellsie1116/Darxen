/* gltktable.c
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

#include "gltktable.h"

#include <GL/gl.h>

G_DEFINE_TYPE(GltkTable, gltk_table, GLTK_TYPE_WIDGET)

#define USING_PRIVATE(obj) GltkTablePrivate* priv = GLTK_TABLE_GET_PRIVATE(obj)
#define GLTK_TABLE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_TABLE, GltkTablePrivate))

enum
{
	LAST_SIGNAL
};

enum
{
	PROP_0,
	PROP_COLS,
	PROP_ROWS,

	N_PROPERTIES
};

typedef struct _GltkTableOptions		GltkTableOptions;
typedef struct _GltkTablePrivate		GltkTablePrivate;

struct _GltkTableOptions
{
	gboolean expand;
	GltkTableCellAlign align;
	int padding;

	int requestSize;
};

struct _GltkTablePrivate
{
	int width;
	int height;

	GltkWidget** widgets;
	GltkTableOptions* xOpts;
	GltkTableOptions* yOpts;

	int expandWidth;
	int expandHeight;
};

//static guint signals[LAST_SIGNAL] = {0,};
static GParamSpec* properties[N_PROPERTIES] = {0,};

static void gltk_table_dispose(GObject* gobject);
static void gltk_table_finalize(GObject* gobject);

static void	gltk_table_set_property	(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec);
static void	gltk_table_get_property	(GObject* object, guint property_id, GValue* value, GParamSpec* pspec);

static void gltk_table_size_request(GltkWidget* widget, GltkSize* size);
static void gltk_table_size_allocate(GltkWidget* widget, GltkAllocation* allocation);
static void gltk_table_set_screen(GltkWidget* widget, GltkScreen* screen);
static void gltk_table_render(GltkWidget* widget);
static gboolean gltk_table_event(GltkWidget* widget, GltkEvent* event);

static void
gltk_table_class_init(GltkTableClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkTablePrivate));
	
	gobject_class->dispose = gltk_table_dispose;
	gobject_class->finalize = gltk_table_finalize;
	gobject_class->set_property = gltk_table_set_property;
	gobject_class->get_property = gltk_table_get_property;

	gltkwidget_class->size_request = gltk_table_size_request;
	gltkwidget_class->size_allocate = gltk_table_size_allocate;
	gltkwidget_class->set_screen = gltk_table_set_screen;
	gltkwidget_class->render = gltk_table_render;
	gltkwidget_class->event = gltk_table_event;

	properties[PROP_COLS] = 
		g_param_spec_int(	"cols", "Cols", "Number of columns",
							1, 100,
							1, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
	
	properties[PROP_ROWS] = 
		g_param_spec_int(	"rows", "Rows", "Number of rows",
							1, 100,
							1, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	g_object_class_install_properties(gobject_class, N_PROPERTIES, properties);
}

static void
gltk_table_init(GltkTable* self)
{
	USING_PRIVATE(self);

	priv->width = 0;
	priv->height = 0;

	priv->widgets = NULL;
	priv->xOpts = NULL;
	priv->yOpts = NULL;

	priv->expandWidth = 0;
	priv->expandHeight = 0;
}

static void
gltk_table_dispose(GObject* gobject)
{
	GltkTable* self = GLTK_TABLE(gobject);
	USING_PRIVATE(self);

	if (priv->widgets)
	{
		int x, y;
		for (y = 0; y < priv->height; y++)
		{
			for (x = 0; x < priv->width; x++)
			{
				GltkWidget* widget = priv->widgets[y*priv->width+x];
				if (!widget)
					continue;

				gltk_widget_unparent(widget);
				gltk_widget_set_screen(widget, NULL);
				g_object_unref(G_OBJECT(widget));
				priv->widgets[y*priv->width+x] = NULL;
			}
		}
		g_free(priv->widgets);
		priv->widgets = NULL;
	}

	G_OBJECT_CLASS(gltk_table_parent_class)->dispose(gobject);
}

static void
gltk_table_finalize(GObject* gobject)
{
	GltkTable* self = GLTK_TABLE(gobject);
	USING_PRIVATE(self);

	g_free(priv->xOpts);
	g_free(priv->yOpts);

	G_OBJECT_CLASS(gltk_table_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_table_new(int cols, int rows)
{
	GObject *gobject = g_object_new(GLTK_TYPE_TABLE, "cols", cols, "rows", rows, NULL);

	return (GltkWidget*)gobject;
}

void
gltk_table_set_col_options(GltkTable* table, int i, GltkTableCellAlign align, gboolean expand, int padding)
{
	g_return_if_fail(GLTK_IS_TABLE(table));
	USING_PRIVATE(table);
	g_return_if_fail(i >= 0 && i < priv->width);

	GltkTableOptions* opts = priv->xOpts + i;

	if (expand && !opts->expand)
		priv->expandWidth++;
	if (!expand && opts->expand)
		priv->expandWidth--;

	opts->align = align;
	opts->expand = expand;
	opts->padding = padding;
}

void
gltk_table_set_row_options(GltkTable* table, int i, GltkTableCellAlign align, gboolean expand, int padding)
{
	g_return_if_fail(GLTK_IS_TABLE(table));
	USING_PRIVATE(table);
	g_return_if_fail(i >= 0 && i < priv->height);
	
	GltkTableOptions* opts = priv->yOpts + i;

	if (expand && !opts->expand)
		priv->expandHeight++;
	if (!expand && opts->expand)
		priv->expandHeight--;

	opts->align = align;
	opts->expand = expand;
	opts->padding = padding;
}

void
gltk_table_set_col_padding(GltkTable* table, int padding)
{
	g_return_if_fail(GLTK_IS_TABLE(table));
	USING_PRIVATE(table);
	
	int i;
	for (i = 0; i < priv->width; i++)
		priv->xOpts[i].padding = padding;

	gltk_widget_layout(GLTK_WIDGET(table));
}

void
gltk_table_set_row_padding(GltkTable* table, int padding)
{
	g_return_if_fail(GLTK_IS_TABLE(table));
	USING_PRIVATE(table);
	
	int i;
	for (i = 0; i < priv->height; i++)
		priv->yOpts[i].padding = padding;

	gltk_widget_layout(GLTK_WIDGET(table));
}


void
gltk_table_insert_widget(GltkTable* table, GltkWidget* widget, int x, int y)
{
	g_return_if_fail(GLTK_IS_TABLE(table));
	g_return_if_fail(GLTK_IS_WIDGET(widget));
	USING_PRIVATE(table);
	g_return_if_fail(x >= 0 && x < priv->width && y >= 0 && y < priv->height);

	GltkWidget* prev = priv->widgets[y*priv->width+x];
	if (prev)
	{
		gltk_widget_unparent(prev);
		gltk_widget_set_screen(prev, NULL);
		g_object_unref(G_OBJECT(prev));
	}

	gltk_widget_set_parent(widget, GLTK_WIDGET(table));
	if (GLTK_WIDGET(table)->screen)
		gltk_widget_set_screen(widget, GLTK_WIDGET(table)->screen);
	priv->widgets[y*priv->width+x] = widget;

	gltk_widget_layout(GLTK_WIDGET(table));
}

void
gltk_table_remove_widget(GltkTable* table, int x, int y)
{
	g_return_if_fail(GLTK_IS_TABLE(table));
	USING_PRIVATE(table);
	GltkWidget* prev = priv->widgets[y*priv->width+x];
	g_return_if_fail(prev);

	gltk_widget_unparent(prev);
	gltk_widget_set_screen(prev, NULL);
	g_object_unref(G_OBJECT(prev));
	priv->widgets[y*priv->width+x] = NULL;
	
	gltk_widget_layout(GLTK_WIDGET(table));
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_table_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec)
{
	GltkTable* self = GLTK_TABLE(object);
	USING_PRIVATE(self);

	switch (property_id)
	{
		case PROP_COLS:
			priv->width = g_value_get_int(value);
			priv->xOpts = g_new0(GltkTableOptions, priv->width);
			if (priv->width && priv->height)
				priv->widgets = g_new0(GltkWidget*, priv->width*priv->height);
			break;
		case PROP_ROWS:
			priv->height = g_value_get_int(value);
			priv->yOpts = g_new0(GltkTableOptions, priv->height);
			if (priv->width && priv->height)
				priv->widgets = g_new0(GltkWidget*, priv->width*priv->height);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void
gltk_table_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec)
{
	GltkTable* self = GLTK_TABLE(object);
	USING_PRIVATE(self);

	switch (property_id)
	{
		case PROP_COLS:
			g_value_set_int(value, priv->width);
			break;
		case PROP_ROWS:
			g_value_set_int(value, priv->height);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void
gltk_table_size_request(GltkWidget* widget, GltkSize* size)
{
	USING_PRIVATE(widget);
	GltkSize* childSizes = (GltkSize*)alloca(sizeof(GltkSize)*priv->width*priv->height);

	size->width = 0;
	size->height = 0;

	int x;
	int y;

	//column pass
	for (x = 0; x < priv->width; x++)
	{
		int width = 0;
		for (y = 0; y < priv->height; y++)
		{
			GltkWidget* widget = priv->widgets[y*priv->width+x];
			GltkSize childSize;
			if (!widget)
				continue;
			gltk_widget_size_request(widget, &childSize);
			width = MAX(width, childSize.width);
			childSizes[y*priv->width+x] = childSize;
		}
		priv->xOpts[x].requestSize = width;
		size->width += width + 2*priv->xOpts[x].padding;
	}

	//row pass
	for (y = 0; y < priv->height; y++)
	{
		int height = 0;
		for (x = 0; x < priv->width; x++)
		{
			GltkWidget* widget = priv->widgets[y*priv->width+x];
			if (!widget)
				continue;
			height = MAX(height, childSizes[y*priv->width+x].height);
		}
		priv->yOpts[y].requestSize = height;
		size->height += height + 2*priv->yOpts[y].padding;
	}

	GLTK_WIDGET_CLASS(gltk_table_parent_class)->size_request(widget, size);
}

static void
gltk_table_size_allocate(GltkWidget* widget, GltkAllocation* allocation)
{
	USING_PRIVATE(widget);

	GltkAllocation* allocations = (GltkAllocation*)alloca(sizeof(GltkAllocation)*priv->width*priv->height);
	GltkSize requisition;
	gltk_widget_size_request(widget, &requisition);

	int dwidth = allocation->width - requisition.width;
	int dheight = allocation->height - requisition.height;

	int x;
	int y;

	if (dwidth < 0)
	{
		//remove width from every widget equally (unless that widget is too small?)
		g_return_if_reached();
	}
	else
	{
		//add extra width to widgets that want to expand
		//position other widgets accordingly
		int extraWidth = priv->expandWidth ? dwidth / priv->expandWidth : 0;
		int posX = 0;
		for (x = 0; x < priv->width; x++)
		{
			GltkTableOptions opts = priv->xOpts[x];
			int allocWidth;
			if (opts.expand)
				allocWidth = opts.requestSize + extraWidth;
			else
				allocWidth = opts.requestSize;
			posX += opts.padding;
			
			for (y = 0; y < priv->height; y++)
			{
				GltkWidget* widget = priv->widgets[y*priv->width+x];
				if (!widget)
					continue;
				const GltkSize* childSize = &widget->requisition;
				GltkAllocation* childAllocation = allocations + (y*priv->width+x);

				switch (opts.align)
				{
					case CELL_ALIGN_LEFT:
						childAllocation->x = posX;
						childAllocation->width = childSize->width;
						break;
					case CELL_ALIGN_CENTER:
						childAllocation->x = posX + (allocWidth - childSize->width) / 2;
						childAllocation->width = childSize->width;
						break;
					case CELL_ALIGN_RIGHT:
						childAllocation->x = posX + allocWidth - childSize->width;
						childAllocation->width = childSize->width;
						break;
					case CELL_ALIGN_JUSTIFY:
						childAllocation->x = posX;
						childAllocation->width = allocWidth;
						break;
				}
			}
			posX += allocWidth + opts.padding;
		}
	}

	//FIXME this is basically a duplicate of above.
	if (dheight < 0)
	{
		//remove height from every widget equally (unless that widget is too small?)
		g_return_if_reached();
	}
	else
	{
		//add extra height to widgets that want to expand
		//position other widgets accordingly
		int extraHeight = priv->expandHeight ? dheight / priv->expandHeight : 0;
		int posY = 0;
		for (y = 0; y < priv->height; y++)
		{
			GltkTableOptions opts = priv->yOpts[y];
			int allocHeight;
			if (opts.expand)
				allocHeight = opts.requestSize + extraHeight;
			else
				allocHeight = opts.requestSize;
			posY += opts.padding;
			
			for (x = 0; x < priv->width; x++)
			{
				GltkWidget* widget = priv->widgets[y*priv->width+x];
				if (!widget)
					continue;
				const GltkSize* childSize = &widget->requisition;
				GltkAllocation* childAllocation = allocations + (y*priv->width+x);

				switch (opts.align)
				{
					case CELL_ALIGN_LEFT:
						childAllocation->y = posY;
						childAllocation->height = childSize->height;
						break;
					case CELL_ALIGN_CENTER:
						childAllocation->y = posY + (allocHeight - childSize->height) / 2;
						childAllocation->height = childSize->height;
						break;
					case CELL_ALIGN_RIGHT:
						childAllocation->y = posY + allocHeight - childSize->height;
						childAllocation->height = childSize->height;
						break;
					case CELL_ALIGN_JUSTIFY:
						childAllocation->y = posY;
						childAllocation->height = allocHeight;
						break;
				}
			}
			posY += allocHeight + opts.padding;
		}
	}

	//set the widget's allocations
	for (y = 0; y < priv->height; y++)
	{
		for (x = 0; x < priv->width; x++)
		{
			GltkWidget* widget = priv->widgets[y*priv->width+x];
			if (!widget)
				continue;

			gltk_widget_size_allocate(widget, allocations[y*priv->width+x]);
		}
	}

	GLTK_WIDGET_CLASS(gltk_table_parent_class)->size_allocate(widget, allocation);
}
static void
gltk_table_set_screen(GltkWidget* widget, GltkScreen* screen)
{
	g_return_if_fail(GLTK_IS_WIDGET(widget));
	USING_PRIVATE(widget);

	int x, y;
	for (y = 0; y < priv->height; y++)
	{
		for (x = 0; x < priv->width; x++)
		{
			GltkWidget* widget = priv->widgets[y*priv->width+x];
			if (!widget)
				continue;

			gltk_widget_set_screen(widget, screen);
		}
	}

	GLTK_WIDGET_CLASS(gltk_table_parent_class)->set_screen(widget, screen);
}

static void
gltk_table_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);
	
	int x, y;
	for (y = 0; y < priv->height; y++)
	{
		for (x = 0; x < priv->width; x++)
		{
			GltkWidget* widget = priv->widgets[y*priv->width+x];
			if (!widget)
				continue;

			GltkAllocation allocation = gltk_widget_get_allocation(widget);
			
			glTranslated(allocation.x, allocation.y, 0);

			gltk_widget_render(widget);

			glTranslated(-allocation.x, -allocation.y, 0);
		}
	}
}

static gboolean
gltk_table_event(GltkWidget* widget, GltkEvent* event)
{
	USING_PRIVATE(widget);

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

			int px;
			int py;
			switch (event->type)
			{
				case GLTK_TOUCH:
					childEvent->touch.positions->x -= allocation.x;
					childEvent->touch.positions->y -= allocation.y;
					px = childEvent->touch.positions->x;
					py = childEvent->touch.positions->y;
					break;
				case GLTK_MULTI_DRAG:
					childEvent->multidrag.center.x -= allocation.x;
					childEvent->multidrag.center.y -= allocation.y;
					px = childEvent->multidrag.center.x;
					py = childEvent->multidrag.center.y;
					break;
				case GLTK_PINCH:
					childEvent->pinch.center.x -= allocation.x;
					childEvent->pinch.center.y -= allocation.y;
					px = childEvent->pinch.center.x;
					py = childEvent->pinch.center.y;
					break;
				case GLTK_ROTATE:
					childEvent->rotate.center.x -= allocation.x;
					childEvent->rotate.center.y -= allocation.y;
					px = childEvent->rotate.center.x;
					py = childEvent->rotate.center.y;
					break;
				default:
					g_assert_not_reached();
					return FALSE;
			}

			int x;
			int y;
			for (y = 0; y < priv->height && !returnValue; y++)
			{
				for (x = 0; x < priv->width && !returnValue; x++)
				{
					GltkWidget* childWidget = priv->widgets[y*priv->width+x];
					if (!childWidget)
						continue;

					GltkAllocation childAllocation = gltk_widget_get_allocation(childWidget);

					if (childAllocation.x < px && childAllocation.x + childAllocation.width > px &&
						childAllocation.y < py && childAllocation.y + childAllocation.height > py)
					{
						returnValue = gltk_widget_send_event(childWidget, childEvent);
					}
				}
			}
			gltk_event_free(childEvent);
		} break;

		default:
			break;
	}

	if (!returnValue)
		returnValue = GLTK_WIDGET_CLASS(gltk_table_parent_class)->event(widget, event);

	return returnValue;
}


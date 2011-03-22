/* gltkwidget.c
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

#include "gltkwidget.h"
#include "gltkmarshal.h"

#include "gltkscreen.h"


G_DEFINE_TYPE(GltkWidget, gltk_widget, G_TYPE_INITIALLY_UNOWNED)

#define USING_PRIVATE(obj) GltkWidgetPrivate* priv = GLTK_WIDGET_GET_PRIVATE(obj)
#define GLTK_WIDGET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_WIDGET, GltkWidgetPrivate))

enum
{
	SIZE_REQUEST,
	SIZE_ALLOCATE,
	EVENT,
	LONG_TOUCH_EVENT,
	TOUCH_EVENT,
	DRAG_EVENT,
	PINCH_EVENT,
	ROTATE_EVENT,
	CLICK_EVENT,
   	LAST_SIGNAL
};

typedef struct _GltkWidgetPrivate		GltkWidgetPrivate;
struct _GltkWidgetPrivate
{
	GltkAllocation allocation;

	gboolean isVisible;
};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_widget_dispose(GObject* gobject);
static void gltk_widget_finalize(GObject* gobject);

static void	gltk_widget_real_size_request(GltkWidget* widget, GltkSize* size);
static void	gltk_widget_real_size_allocate(GltkWidget* widget, GltkAllocation* allocation);
static gboolean	gltk_widget_real_event(GltkWidget* widget, GltkEvent* event);

static void	gltk_widget_set_screen_default(GltkWidget* widget, GltkScreen* screen);
static void gltk_widget_render_default(GltkWidget* widget);

static void
gltk_widget_class_init(GltkWidgetClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkWidgetPrivate));
	
	signals[SIZE_REQUEST] = 
		g_signal_new(	"size-request",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET(GltkWidgetClass, size_request),
						NULL, NULL,
						g_cclosure_marshal_VOID__POINTER,
						G_TYPE_NONE, 1,
						G_TYPE_POINTER);

	signals[SIZE_ALLOCATE] = 
		g_signal_new(	"size-allocate",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET(GltkWidgetClass, size_allocate),
						NULL, NULL,
						g_cclosure_marshal_VOID__POINTER,
						G_TYPE_NONE, 1,
						G_TYPE_POINTER);
	
	signals[EVENT] = 
		g_signal_new(	"event",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkWidgetClass, event),
						gltk_accum_event, NULL,
						g_cclosure_user_marshal_BOOLEAN__POINTER,
						G_TYPE_BOOLEAN, 1,
						G_TYPE_POINTER);

	signals[LONG_TOUCH_EVENT] = 
		g_signal_new(	"long-touch-event",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkWidgetClass, long_touch_event),
						gltk_accum_event, NULL,
						g_cclosure_user_marshal_BOOLEAN__POINTER,
						G_TYPE_BOOLEAN, 1,
						G_TYPE_POINTER);

	signals[TOUCH_EVENT] = 
		g_signal_new(	"touch-event",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkWidgetClass, touch_event),
						gltk_accum_event, NULL,
						g_cclosure_user_marshal_BOOLEAN__POINTER,
						G_TYPE_BOOLEAN, 1,
						G_TYPE_POINTER);

	signals[DRAG_EVENT] = 
		g_signal_new(	"drag-event",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkWidgetClass, drag_event),
						gltk_accum_event, NULL,
						g_cclosure_user_marshal_BOOLEAN__POINTER,
						G_TYPE_BOOLEAN, 1,
						G_TYPE_POINTER);

	signals[PINCH_EVENT] = 
		g_signal_new(	"pinch-event",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkWidgetClass, pinch_event),
						gltk_accum_event, NULL,
						g_cclosure_user_marshal_BOOLEAN__POINTER,
						G_TYPE_BOOLEAN, 1,
						G_TYPE_POINTER);

	signals[ROTATE_EVENT] = 
		g_signal_new(	"rotate-event",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkWidgetClass, rotate_event),
						gltk_accum_event, NULL,
						g_cclosure_user_marshal_BOOLEAN__POINTER,
						G_TYPE_BOOLEAN, 1,
						G_TYPE_POINTER);

	signals[CLICK_EVENT] = 
		g_signal_new(	"click-event",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkWidgetClass, click_event),
						gltk_accum_event, NULL,
						g_cclosure_user_marshal_BOOLEAN__POINTER,
						G_TYPE_BOOLEAN, 1,
						G_TYPE_POINTER);

	gobject_class->dispose = gltk_widget_dispose;
	gobject_class->finalize = gltk_widget_finalize;

	klass->size_request = gltk_widget_real_size_request;
	klass->size_allocate = gltk_widget_real_size_allocate;
	klass->event = gltk_widget_real_event;
	klass->long_touch_event = NULL;
	klass->touch_event = NULL;
	klass->drag_event = NULL;
	klass->pinch_event = NULL;
	klass->rotate_event = NULL;
	klass->click_event = NULL;

	klass->set_screen = gltk_widget_set_screen_default;
	klass->render = gltk_widget_render_default;
}


static void
gltk_widget_init(GltkWidget* self)
{
	USING_PRIVATE(self);

	static GltkAllocation initialAllocation = {0, 0, -1, -1};

	self->screen = NULL;
	self->parentWidget = NULL;

	priv->allocation = initialAllocation;
	priv->isVisible = TRUE;
}

static void
gltk_widget_dispose(GObject* gobject)
{
	GltkWidget* self = GLTK_WIDGET(gobject);

	if (self->parentWidget)
	{
		g_object_unref(G_OBJECT(self->parentWidget));
		self->parentWidget = NULL;
	}

	G_OBJECT_CLASS(gltk_widget_parent_class)->dispose(gobject);
}

static void
gltk_widget_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_widget_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_widget_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_WIDGET, NULL);

	return (GltkWidget*)gobject;
}

GltkWidget*
gltk_widget_get_parent(GltkWidget* widget)
{
	g_return_val_if_fail(GLTK_IS_WIDGET(widget), NULL);

	return widget->parentWidget;
}

void
gltk_widget_set_parent(GltkWidget* widget, GltkWidget* parent)
{
	g_return_if_fail(GLTK_IS_WIDGET(widget));
	g_return_if_fail(GLTK_IS_WIDGET(parent));
	g_return_if_fail(widget != parent);
	g_return_if_fail(!widget->parentWidget);

	g_object_ref(parent);
	widget->parentWidget = parent;
}

void
gltk_widget_unparent(GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_WIDGET(widget));
	g_return_if_fail(GLTK_IS_WIDGET(widget->parentWidget));

	g_object_unref(G_OBJECT(widget->parentWidget));
	widget->parentWidget = NULL;
}


void
gltk_widget_set_screen(GltkWidget* widget, GltkScreen* screen)
{
	g_return_if_fail(GLTK_IS_WIDGET(widget));
	g_return_if_fail(!screen || GLTK_IS_SCREEN(screen));

	GLTK_WIDGET_GET_CLASS(widget)->set_screen(widget, screen);
}

void
gltk_widget_set_visible(GltkWidget* widget, gboolean visible)
{
	g_return_if_fail(GLTK_IS_WIDGET(widget));
	USING_PRIVATE(widget);

	if (priv->isVisible != (priv->isVisible = visible))
		gltk_widget_layout(widget);
}

gboolean
gltk_widget_get_visible(GltkWidget* widget)
{
	g_return_val_if_fail(GLTK_IS_WIDGET(widget), FALSE);
	USING_PRIVATE(widget);

	return priv->isVisible;
}


void
gltk_widget_size_request(GltkWidget* widget, GltkSize* size)
{
	g_signal_emit(G_OBJECT(widget), signals[SIZE_REQUEST], 0, size);
}

void
gltk_widget_size_allocate(GltkWidget* widget, GltkAllocation allocation)
{
	g_signal_emit(G_OBJECT(widget), signals[SIZE_ALLOCATE], 0, &allocation);
}

GltkAllocation
gltk_widget_get_allocation(GltkWidget* widget)
{
	static GltkAllocation empty = {-1, -1, -1, -1};
	g_return_val_if_fail(GLTK_IS_WIDGET(widget), empty);
	USING_PRIVATE(widget);

	return priv->allocation;
}

GltkAllocation
gltk_widget_get_global_allocation(GltkWidget* widget)
{
	GltkAllocation res = {0,};
	g_return_val_if_fail(GLTK_IS_WIDGET(widget), res);

	res = gltk_widget_get_allocation(widget);

	while ((widget = gltk_widget_get_parent(widget)))
	{
		GltkAllocation parentAllocation = gltk_widget_get_allocation(widget);
		res.x += parentAllocation.x;
		res.y += parentAllocation.y;
	}
	return res;
}

void
gltk_widget_invalidate(GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_WIDGET(widget));

	if (widget->screen)
		gltk_screen_invalidate(widget->screen);
}

void
gltk_widget_layout(GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_WIDGET(widget));

	if (widget->screen)
		gltk_screen_layout(widget->screen);
}

void
gltk_widget_render(GltkWidget* widget)
{
	g_return_if_fail(GLTK_IS_WIDGET(widget));

	GLTK_WIDGET_GET_CLASS(widget)->render(widget);
}

gboolean
gltk_widget_send_event(GltkWidget* widget, GltkEvent* event)
{
	g_return_val_if_fail(GLTK_IS_WIDGET(widget), FALSE);
	gboolean returnValue;
	g_signal_emit(G_OBJECT(widget), signals[EVENT], 0, event, &returnValue);
	return returnValue;
}

GQuark
gltk_widget_error_quark()
{
	return g_quark_from_static_string("gltk-widget-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_widget_real_size_request(GltkWidget* widget, GltkSize* size)
{
	g_return_if_fail(GLTK_IS_WIDGET(widget));

	widget->requisition = *size;
}

static void
gltk_widget_real_size_allocate(GltkWidget* widget, GltkAllocation* allocation)
{
	USING_PRIVATE(widget);

	//g_message("Widget allocation: %3d %3d %3d %3d", allocation->x, allocation->y, allocation->width, allocation->height);

	priv->allocation = *allocation;
}

static gboolean
gltk_widget_real_event(GltkWidget* widget, GltkEvent* event)
{
	gboolean returnValue = FALSE;

	GltkWidget* pWidget = widget;
	for (pWidget = widget; pWidget; pWidget = pWidget->parentWidget)
	{
		gboolean eventReturnValue = FALSE;
		switch (event->type)
		{
			case GLTK_LONG_TOUCH:
				g_signal_emit(G_OBJECT(pWidget), signals[LONG_TOUCH_EVENT], 0, event, &eventReturnValue);
				break;
			case GLTK_TOUCH:
				g_signal_emit(G_OBJECT(pWidget), signals[TOUCH_EVENT], 0, event, &eventReturnValue);
				break;
			case GLTK_DRAG:
				g_signal_emit(G_OBJECT(pWidget), signals[DRAG_EVENT], 0, event, &eventReturnValue);
				break;
			case GLTK_PINCH:
				g_signal_emit(G_OBJECT(pWidget), signals[PINCH_EVENT], 0, event, &eventReturnValue);
				break;
			case GLTK_ROTATE:
				g_signal_emit(G_OBJECT(pWidget), signals[ROTATE_EVENT], 0, event, &eventReturnValue);
				break;
			case GLTK_CLICK:
				g_signal_emit(G_OBJECT(pWidget), signals[CLICK_EVENT], 0, event, &eventReturnValue);
				break;
			default:
				g_warning("Unhandled event type: %i", event->type);
		}
		if (!returnValue)
			returnValue = eventReturnValue;

		//only send long touches to 1 widget
		if (returnValue && event->type == GLTK_LONG_TOUCH)
			break;
	}

	return returnValue;
}

static void
gltk_widget_set_screen_default(GltkWidget* widget, GltkScreen* screen)
{
	widget->screen = screen;
}

static void
gltk_widget_render_default(GltkWidget* widget)
{
	g_warning("A widget forgot to override the render method");
}


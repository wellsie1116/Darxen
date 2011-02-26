/* gltkwidget.h
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

#ifndef GLTKWIDGET_H_K1ZIAA3N
#define GLTKWIDGET_H_K1ZIAA3N

#include <glib-object.h>

#include "gltkevents.h"

#include "gltkwindow.h"

G_BEGIN_DECLS

typedef struct
{
	int width;
	int height;
} GltkSize;

typedef struct
{
	int x;
	int y;
	int width;
	int height;
} GltkAllocation;

#define GLTK_WIDGET_ERROR gltk_widget_error_quark()

#define GLTK_TYPE_WIDGET				(gltk_widget_get_type())
#define GLTK_WIDGET(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_WIDGET, GltkWidget))
#define GLTK_WIDGET_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_WIDGET, GltkWidgetClass))
#define GLTK_IS_WIDGET(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_WIDGET))
#define GLTK_IS_WIDGET_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_WIDGET))
#define GLTK_WIDGET_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_WIDGET, GltkWidgetClass))

#ifndef GLTK_WIDGET_DEF
#define GLTK_WIDGET_DEF
typedef struct _GltkWidget			GltkWidget;
#endif
typedef struct _GltkWidgetClass		GltkWidgetClass;

struct _GltkWidget
{
	GObject parent;

	GltkWindow* window;

	GltkWidget* parentWidget;

	GltkSize requisition;
};

struct _GltkWidgetClass
{
	GObjectClass parent_class;
	
	/* signals */
	void (*size_request)	(	GltkWidget* widget,
								GltkSize* size);

	void (*size_allocate)	(	GltkWidget* widget,
								GltkAllocation* allocation);

	gboolean (*event)		(	GltkWidget* widget,
								GltkEvent* event);

	gboolean (*touch_event)	(	GltkWidget* widget,
								GltkEventTouch* event);

	gboolean (*click_event)	(	GltkWidget* widget,
								GltkEventClick* event);

	/* virtual funcs */
	void (*set_window)		(	GltkWidget* widget,
								GltkWindow* window);

	void (*render)			(	GltkWidget* widget);
};

typedef enum
{
	GLTK_WIDGET_ERROR_FAILED
} GltkWidgetError;

GType			gltk_widget_get_type		() G_GNUC_CONST;
GltkWidget*		gltk_widget_new				();

GltkWidget*		gltk_widget_get_parent		(GltkWidget* widget);
void			gltk_widget_set_parent		(GltkWidget* widget, GltkWidget* parent);
void			gltk_widget_unparent		(GltkWidget* widget);
void			gltk_widget_set_window		(GltkWidget* widget, GltkWindow* window);
void			gltk_widget_size_request	(GltkWidget* widget, GltkSize* size);
void			gltk_widget_size_allocate	(GltkWidget* widget, GltkAllocation allocation);
GltkAllocation	gltk_widget_get_allocation	(GltkWidget* widget);
void			gltk_widget_invalidate		(GltkWidget* widget);

void			gltk_widget_render			(GltkWidget* widget);

gboolean		gltk_widget_send_event		(GltkWidget* widget, GltkEvent* event);

GQuark			gltk_widget_error_quark		();

G_END_DECLS

#endif


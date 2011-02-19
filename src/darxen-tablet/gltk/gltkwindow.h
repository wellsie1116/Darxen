/* gltkwindow.h
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

#ifndef GLTKWINDOW_H_IHO82B7B
#define GLTKWINDOW_H_IHO82B7B

#include <glib-object.h>

#include "gltkwidget.h"

G_BEGIN_DECLS

#define GLTK_WINDOW_ERROR gltk_window_error_quark()

#define GLTK_TYPE_WINDOW				(gltk_window_get_type())
#define GLTK_WINDOW(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_WINDOW, GltkWindow))
#define GLTK_WINDOW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_WINDOW, GltkWindowClass))
#define GLTK_IS_WINDOW(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_WINDOW))
#define GLTK_IS_WINDOW_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_WINDOW))
#define GLTK_WINDOW_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_WINDOW, GltkWindowClass))

typedef struct _GltkWindow			GltkWindow;
typedef struct _GltkWindowClass		GltkWindowClass;

struct _GltkWindow
{
	GObject parent;
};

struct _GltkWindowClass
{
	GObjectClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	GLTK_WINDOW_ERROR_FAILED
} GltkWindowError;

GType			gltk_window_get_type	() G_GNUC_CONST;
GltkWindow*		gltk_window_new			();

/* Public functions here */
void			gltk_window_set_size	(GltkWindow* window, int width, int height);
void			gltk_window_render		(GltkWindow* window);
void			gltk_window_set_root	(GltkWindow* window, GltkWidget* widget);

GQuark			gltk_window_error_quark	();

G_END_DECLS

#endif


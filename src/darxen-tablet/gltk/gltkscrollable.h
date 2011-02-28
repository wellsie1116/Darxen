/* gltkscrollable.h
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

#ifndef GLTKSCROLLABLE_H_AB5Z9TH3
#define GLTKSCROLLABLE_H_AB5Z9TH3

#include "gltkwidget.h"

G_BEGIN_DECLS

#define GLTK_SCROLLABLE_ERROR gltk_scrollable_error_quark()

#define GLTK_TYPE_SCROLLABLE				(gltk_scrollable_get_type())
#define GLTK_SCROLLABLE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_SCROLLABLE, GltkScrollable))
#define GLTK_SCROLLABLE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_SCROLLABLE, GltkScrollableClass))
#define GLTK_IS_SCROLLABLE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_SCROLLABLE))
#define GLTK_IS_SCROLLABLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_SCROLLABLE))
#define GLTK_SCROLLABLE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_SCROLLABLE, GltkScrollableClass))

typedef struct _GltkScrollable			GltkScrollable;
typedef struct _GltkScrollableClass		GltkScrollableClass;

struct _GltkScrollable
{
	GltkWidget parent;
};

struct _GltkScrollableClass
{
	GltkWidgetClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	GLTK_SCROLLABLE_ERROR_FAILED
} GltkScrollableError;

GType			gltk_scrollable_get_type	() G_GNUC_CONST;
GltkWidget*		gltk_scrollable_new			();

void			gltk_scrollable_set_widget	(GltkScrollable* scrollable, GltkWidget* widget);

GQuark			gltk_scrollable_error_quark	();

G_END_DECLS

#endif


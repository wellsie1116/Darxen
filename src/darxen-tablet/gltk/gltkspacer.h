/* gltkspacer.h
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

#ifndef GLTKSPACER_H_NG2KD8MB
#define GLTKSPACER_H_NG2KD8MB

#include "gltkwidget.h"

G_BEGIN_DECLS

#define GLTK_TYPE_SPACER				(gltk_spacer_get_type())
#define GLTK_SPACER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_SPACER, GltkSpacer))
#define GLTK_SPACER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_SPACER, GltkSpacerClass))
#define GLTK_IS_SPACER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_SPACER))
#define GLTK_IS_SPACER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_SPACER))
#define GLTK_SPACER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_SPACER, GltkSpacerClass))

typedef struct _GltkSpacer			GltkSpacer;
typedef struct _GltkSpacerClass		GltkSpacerClass;

struct _GltkSpacer
{
	GltkWidget parent;
};

struct _GltkSpacerClass
{
	GltkWidgetClass parent_class;
};

GType			gltk_spacer_get_type	() G_GNUC_CONST;
GltkWidget*		gltk_spacer_new			(int width, int height);

G_END_DECLS

#endif


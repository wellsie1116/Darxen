/* gltkhbox.h
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

#ifndef GLTKHBOX_H_N3G1RQP6
#define GLTKHBOX_H_N3G1RQP6

#include <glib-object.h>

#include "gltkbox.h"

G_BEGIN_DECLS

#define GLTK_HBOX_ERROR				gltk_hbox_error_quark()

#define GLTK_TYPE_HBOX				(gltk_hbox_get_type())
#define GLTK_HBOX(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_HBOX, GltkHBox))
#define GLTK_HBOX_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_HBOX, GltkHBoxClass))
#define GLTK_IS_HBOX(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_HBOX))
#define GLTK_IS_HBOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_HBOX))
#define GLTK_HBOX_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_HBOX, GltkHBoxClass))

typedef struct _GltkHBox			GltkHBox;
typedef struct _GltkHBoxClass		GltkHBoxClass;

struct _GltkHBox
{
	GltkBox parent;
};

struct _GltkHBoxClass
{
	GltkBoxClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	GLTK_HBOX_ERROR_FAILED
} GltkHBoxError;

GType			gltk_hbox_get_type	() G_GNUC_CONST;
GltkWidget*		gltk_hbox_new		(int spacing);

/* Public functions here */

GQuark			gltk_hbox_error_quark	();

G_END_DECLS

#endif


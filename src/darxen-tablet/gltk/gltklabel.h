/* gltklabel.h
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

#ifndef GLTKLABEL_H_OWJUAREV
#define GLTKLABEL_H_OWJUAREV

#include <glib-object.h>

G_BEGIN_DECLS

#define GLTK_LABEL_ERROR gltk_label_error_quark()

#define GLTK_TYPE_LABEL				(gltk_label_get_type())
#define GLTK_LABEL(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_LABEL, GltkLabel))
#define GLTK_LABEL_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_LABEL, GltkLabelClass))
#define GLTK_IS_LABEL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_LABEL))
#define GLTK_IS_LABEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_LABEL))
#define GLTK_LABEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_LABEL, GltkLabelClass))

typedef struct _GltkLabel			GltkLabel;
typedef struct _GltkLabelClass		GltkLabelClass;

struct _GltkLabel
{
	GltkWidget parent;
};

struct _GltkLabelClass
{
	GltkWidgetClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	GLTK_LABEL_ERROR_FAILED
} GltkLabelError;

GType			gltk_label_get_type	() G_GNUC_CONST;
GltkWidget*		gltk_label_new		(const gchar* text);

void			gltk_label_set_text	(const gchar* text);

GQuark			gltk_label_error_quark	();

G_END_DECLS

#endif

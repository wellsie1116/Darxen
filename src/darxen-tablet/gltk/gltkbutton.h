/* gltkbutton.h
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

#ifndef GLTKBUTTON_H_G15GHWDE
#define GLTKBUTTON_H_G15GHWDE

#include "gltkwidget.h"

G_BEGIN_DECLS

#define GLTK_BUTTON_ERROR gltk_button_error_quark()

#define GLTK_TYPE_BUTTON				(gltk_button_get_type())
#define GLTK_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_BUTTON, GltkButton))
#define GLTK_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_BUTTON, GltkButtonClass))
#define GLTK_IS_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_BUTTON))
#define GLTK_IS_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_BUTTON))
#define GLTK_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_BUTTON, GltkButtonClass))

typedef struct _GltkButton			GltkButton;
typedef struct _GltkButtonClass		GltkButtonClass;

struct _GltkButton
{
	GltkWidget parent;
};

struct _GltkButtonClass
{
	GltkWidgetClass parent_class;
	
	/* signals */

	/* virtual funcs */
};

typedef enum
{
	GLTK_BUTTON_ERROR_FAILED
} GltkButtonError;

GType			gltk_button_get_type	() G_GNUC_CONST;
GltkWidget*		gltk_button_new			(const gchar* text);

const gchar*	gltk_button_get_text	(GltkButton* button);

GQuark			gltk_button_error_quark	();

G_END_DECLS

#endif


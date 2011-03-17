/* gltkkeyboard.h
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

#ifndef GLTKKEYBOARD_H_DB2Z84K8
#define GLTKKEYBOARD_H_DB2Z84K8

#include "gltkvbox.h"

G_BEGIN_DECLS

#define GLTK_KEYBOARD_ERROR gltk_keyboard_error_quark()

#define GLTK_TYPE_KEYBOARD				(gltk_keyboard_get_type())
#define GLTK_KEYBOARD(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_KEYBOARD, GltkKeyboard))
#define GLTK_KEYBOARD_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_KEYBOARD, GltkKeyboardClass))
#define GLTK_IS_KEYBOARD(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_KEYBOARD))
#define GLTK_IS_KEYBOARD_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_KEYBOARD))
#define GLTK_KEYBOARD_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_KEYBOARD, GltkKeyboardClass))

typedef struct _GltkKeyboard			GltkKeyboard;
typedef struct _GltkKeyboardClass		GltkKeyboardClass;

struct _GltkKeyboard
{
	GltkVBox parent;

};

struct _GltkKeyboardClass
{
	GltkVBoxClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	GLTK_KEYBOARD_ERROR_FAILED
} GltkKeyboardError;

GType			gltk_keyboard_get_type		() G_GNUC_CONST;
GltkWidget*		gltk_keyboard_new			(const gchar* text);

const gchar*	gltk_keyboard_get_text		(GltkKeyboard* keyboard);

GQuark			gltk_keyboard_error_quark	();

G_END_DECLS

#endif


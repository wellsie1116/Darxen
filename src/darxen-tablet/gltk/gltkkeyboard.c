/* gltkkeyboard.c
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

#include "gltkkeyboard.h"

G_DEFINE_TYPE(GltkKeyboard, gltk_keyboard, GLTK_TYPE_VBOX)

#define USING_PRIVATE(obj) GltkKeyboardPrivate* priv = GLTK_KEYBOARD_GET_PRIVATE(obj)
#define GLTK_KEYBOARD_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_KEYBOARD, GltkKeyboardPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkKeyboardPrivate		GltkKeyboardPrivate;
struct _GltkKeyboardPrivate
{
	int dummy;
};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_keyboard_dispose(GObject* gobject);
static void gltk_keyboard_finalize(GObject* gobject);

static void
gltk_keyboard_class_init(GltkKeyboardClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkKeyboardPrivate));
	
	gobject_class->dispose = gltk_keyboard_dispose;
	gobject_class->finalize = gltk_keyboard_finalize;
}

static void
gltk_keyboard_init(GltkKeyboard* self)
{
	USING_PRIVATE(self);

	gltk_box_append_widget(GLTK_BOX(self), gltk_label_new("I am a keyboard"), TRUE, TRUE);
}

static void
gltk_keyboard_dispose(GObject* gobject)
{
	GltkKeyboard* self = GLTK_KEYBOARD(gobject);
	USING_PRIVATE(self);

	//free and release references

	G_OBJECT_CLASS(gltk_keyboard_parent_class)->dispose(gobject);
}

static void
gltk_keyboard_finalize(GObject* gobject)
{
	GltkKeyboard* self = GLTK_KEYBOARD(gobject);
	USING_PRIVATE(self);

	//free memory

	G_OBJECT_CLASS(gltk_keyboard_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_keyboard_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_KEYBOARD, NULL);
	GltkKeyboard* self = GLTK_KEYBOARD(gobject);

	USING_PRIVATE(self);

	return (GltkWidget*)gobject;
}


GQuark
gltk_keyboard_error_quark()
{
	return g_quark_from_static_string("gltk-keyboard-error-quark");
}

/*********************
 * Private Functions *
 *********************/


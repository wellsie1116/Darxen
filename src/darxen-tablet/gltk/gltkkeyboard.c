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

#include "gltkhbox.h"
#include "gltkbutton.h"
#include "gltklabel.h"

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
	GltkWidget* label;
};

//static guint signals[LAST_SIGNAL] = {0,};

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

static gboolean
key_clicked(GltkWidget* button, GltkEventClick* event, GltkKeyboard* keyboard)
{
	USING_PRIVATE(keyboard);

	GltkLabel* label = GLTK_LABEL(priv->label);

	gchar* newText = g_strdup_printf("%s%c", label->text, GLTK_BUTTON(button)->text[0]);
	g_free(label->text);
	label->text = newText;

	gltk_widget_invalidate(GLTK_WIDGET(priv->label));
	return TRUE;
}

static gboolean
keyBackspace_clicked(GltkWidget* button, GltkEventClick* event, GltkKeyboard* keyboard)
{
	USING_PRIVATE(keyboard);

	GltkLabel* label = GLTK_LABEL(priv->label);
	int len = strlen(label->text);
	if (!len)
		return TRUE;

	label->text[len-1] = '\0';

	gltk_widget_invalidate(GLTK_WIDGET(priv->label));
	return TRUE;
}

static void
add_key_sized(GltkKeyboard* keyboard, GltkBox* box, char key, char shiftKey, float width)
{
	char txt[2] = {key, '\0'};
	GltkWidget* button = gltk_button_new(txt);
	gltk_box_append_widget(box, button, FALSE, FALSE);
	g_signal_connect(button, "click-event", (GCallback)key_clicked, keyboard);
}

static void
add_key(GltkKeyboard* keyboard, GltkBox* box, char key, char shiftKey)
{
	add_key_sized(keyboard, box, key, shiftKey, 1.0f);
}

static void
add_special_key(GltkKeyboard* keyboard, GltkBox* box, const char* txt, GCallback callback, float width)
{
	GltkWidget* button = gltk_button_new(txt);
	gltk_box_append_widget(box, button, FALSE, FALSE);
	g_signal_connect(button, "click-event", callback, keyboard);
}

GltkWidget*
gltk_keyboard_new(const gchar* text)
{
	GObject *gobject = g_object_new(GLTK_TYPE_KEYBOARD, NULL);
	GltkKeyboard* self = GLTK_KEYBOARD(gobject);

	USING_PRIVATE(self);

	GltkWidget* hboxTop = gltk_hbox_new();
	{
		priv->label = gltk_label_new(text);
		g_object_ref(priv->label);
		gltk_label_set_draw_border(GLTK_LABEL(priv->label), TRUE);
		gltk_box_append_widget(GLTK_BOX(self), priv->label, TRUE, TRUE);
	}
	
	GltkWidget* hboxNumbers = gltk_hbox_new();
	{
		add_key_sized(self, GLTK_BOX(hboxNumbers), '1', '!', 0.5f);
		add_key(self, GLTK_BOX(hboxNumbers), '2', '@');
		add_key(self, GLTK_BOX(hboxNumbers), '3', '#');
		add_key(self, GLTK_BOX(hboxNumbers), '4', '$');
		add_key(self, GLTK_BOX(hboxNumbers), '5', '%');
		add_key(self, GLTK_BOX(hboxNumbers), '6', '^');
		add_key(self, GLTK_BOX(hboxNumbers), '7', '&');
		add_key(self, GLTK_BOX(hboxNumbers), '8', '*');
		add_key(self, GLTK_BOX(hboxNumbers), '9', '(');
		add_key(self, GLTK_BOX(hboxNumbers), '0', ')');
		add_key(self, GLTK_BOX(hboxNumbers), '-', '_');
		add_key(self, GLTK_BOX(hboxNumbers), '=', '+');
		add_special_key(self, GLTK_BOX(hboxNumbers), "<-", (GCallback)keyBackspace_clicked, 2.0f);
		//backspace
	}

	GltkWidget* hboxRow1 = gltk_hbox_new();
	{
		//tab
		add_key(self, GLTK_BOX(hboxRow1), 'q', 'Q');
		add_key(self, GLTK_BOX(hboxRow1), 'w', 'W');
		add_key(self, GLTK_BOX(hboxRow1), 'e', 'E');
		add_key(self, GLTK_BOX(hboxRow1), 'r', 'R');
		add_key(self, GLTK_BOX(hboxRow1), 't', 'T');
		add_key(self, GLTK_BOX(hboxRow1), 'y', 'Y');
		add_key(self, GLTK_BOX(hboxRow1), 'u', 'U');
		add_key(self, GLTK_BOX(hboxRow1), 'i', 'I');
		add_key(self, GLTK_BOX(hboxRow1), 'o', 'O');
		add_key(self, GLTK_BOX(hboxRow1), 'p', 'P');
		add_key(self, GLTK_BOX(hboxRow1), '[', '{');
		add_key(self, GLTK_BOX(hboxRow1), ']', '}');
		add_key(self, GLTK_BOX(hboxRow1), '\\', '|');
	}
	
	GltkWidget* hboxRow2 = gltk_hbox_new();
	{
		//caps lock
		add_key(self, GLTK_BOX(hboxRow2), 'a', 'A');
		add_key(self, GLTK_BOX(hboxRow2), 's', 'S');
		add_key(self, GLTK_BOX(hboxRow2), 'd', 'D');
		add_key(self, GLTK_BOX(hboxRow2), 'f', 'F');
		add_key(self, GLTK_BOX(hboxRow2), 'g', 'G');
		add_key(self, GLTK_BOX(hboxRow2), 'h', 'H');
		add_key(self, GLTK_BOX(hboxRow2), 'j', 'J');
		add_key(self, GLTK_BOX(hboxRow2), 'k', 'K');
		add_key(self, GLTK_BOX(hboxRow2), 'l', 'L');
		add_key(self, GLTK_BOX(hboxRow2), ';', ':');
		add_key(self, GLTK_BOX(hboxRow2), '\'', '"');
	}
	
	GltkWidget* hboxRow3 = gltk_hbox_new();
	{
		//shift
		add_key(self, GLTK_BOX(hboxRow3), 'z', 'Z');
		add_key(self, GLTK_BOX(hboxRow3), 'x', 'X');
		add_key(self, GLTK_BOX(hboxRow3), 'c', 'C');
		add_key(self, GLTK_BOX(hboxRow3), 'v', 'V');
		add_key(self, GLTK_BOX(hboxRow3), 'b', 'B');
		add_key(self, GLTK_BOX(hboxRow3), 'n', 'N');
		add_key(self, GLTK_BOX(hboxRow3), 'm', 'M');
		add_key(self, GLTK_BOX(hboxRow3), ',', '<');
		add_key(self, GLTK_BOX(hboxRow3), '.', '>');
		add_key(self, GLTK_BOX(hboxRow3), '/', '?');
	}

	gltk_box_append_widget(GLTK_BOX(self), hboxTop, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxNumbers, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxRow1, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxRow2, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxRow3, FALSE, FALSE);

	return (GltkWidget*)gobject;
}

const gchar*
gltk_keyboard_get_text(GltkKeyboard* keyboard)
{
	g_return_val_if_fail(GLTK_IS_KEYBOARD(keyboard), NULL);
	USING_PRIVATE(keyboard);
	
	return GLTK_LABEL(priv->label)->text;
}

GQuark
gltk_keyboard_error_quark()
{
	return g_quark_from_static_string("gltk-keyboard-error-quark");
}

/*********************
 * Private Functions *
 *********************/


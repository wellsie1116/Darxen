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
#include "gltktogglebutton.h"
#include "gltklabel.h"

#include <string.h>

G_DEFINE_TYPE(GltkKeyboard, gltk_keyboard, GLTK_TYPE_VBOX)

#define USING_PRIVATE(obj) GltkKeyboardPrivate* priv = GLTK_KEYBOARD_GET_PRIVATE(obj)
#define GLTK_KEYBOARD_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_KEYBOARD, GltkKeyboardPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkKeyboardPrivate		GltkKeyboardPrivate;
typedef struct _KeyboardKey				KeyboardKey;

struct _KeyboardKey
{
	char key;
	char shiftKey;
	float width;
	GltkWidget* button;
};

struct _GltkKeyboardPrivate
{
	GltkWidget* label;

	GltkWidget* shift;
	GltkWidget* caps;

	GSList* keys; //KeyboardKey
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

	if (priv->label)
	{
		g_object_unref(priv->label);
		priv->label = NULL;
	}

	if (priv->shift)
	{
		g_object_unref(priv->shift);
		priv->shift = NULL;
	}

	if (priv->caps)
	{
		g_object_unref(priv->caps);
		priv->caps = NULL;
	}

	if (priv->keys)
	{
		GSList* pKeys = priv->keys;
		while (pKeys)
		{
			g_free(pKeys->data);
			pKeys = pKeys->next;
		}
		g_slist_free(priv->keys);
		priv->keys = NULL;
	}

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

static void
set_shift(GltkKeyboard* keyboard, gboolean value)
{
	USING_PRIVATE(keyboard);

	GSList* pKeys = priv->keys;
	char txt[2] = {0,};
	while (pKeys)
	{
		KeyboardKey* key = (KeyboardKey*)pKeys->data;
	
		txt[0] = value ? key->shiftKey : key->key;
		GLTK_BUTTON(key->button)->text = g_strdup(txt);
	
		pKeys = pKeys->next;
	}
}

static gboolean
key_clicked(GltkWidget* button, GltkEventClick* event, GltkKeyboard* keyboard)
{
	USING_PRIVATE(keyboard);

	GltkLabel* label = GLTK_LABEL(priv->label);

	gchar* newText = g_strdup_printf("%s%c", label->text, GLTK_BUTTON(button)->text[0]);
	g_free(label->text);
	label->text = newText;
	
	if (gltk_toggle_button_is_toggled(GLTK_TOGGLE_BUTTON(priv->shift)))
	{
		gltk_toggle_button_set_toggled(GLTK_TOGGLE_BUTTON(priv->shift), FALSE);
		set_shift(keyboard, gltk_toggle_button_is_toggled(GLTK_TOGGLE_BUTTON(priv->shift)));
	}

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

static gboolean
keyBackspace_longTouched(GltkWidget* button, GltkEventClick* event, GltkKeyboard* keyboard)
{
	USING_PRIVATE(keyboard);

	GltkLabel* label = GLTK_LABEL(priv->label);
	int len = strlen(label->text);
	if (!len)
		return TRUE;

	g_free(label->text);
	label->text = g_strdup("");

	gltk_widget_invalidate(GLTK_WIDGET(priv->label));
	return TRUE;
}

static gboolean
keyCaps_clicked(GltkWidget* button, GltkEventClick* event, GltkKeyboard* keyboard)
{
	USING_PRIVATE(keyboard);

	gboolean isShift = gltk_toggle_button_is_toggled(GLTK_TOGGLE_BUTTON(priv->shift));
	gboolean isCaps = gltk_toggle_button_is_toggled(GLTK_TOGGLE_BUTTON(priv->caps));

	if (isCaps && isShift)
	{
		gltk_toggle_button_set_toggled(GLTK_TOGGLE_BUTTON(priv->shift), FALSE);
	}

	set_shift(keyboard, isCaps);

	gltk_widget_invalidate(GLTK_WIDGET(keyboard));
	return FALSE;
}

static gboolean
keyShift_clicked(GltkWidget* button, GltkEventClick* event, GltkKeyboard* keyboard)
{
	USING_PRIVATE(keyboard);

	gboolean isShift = gltk_toggle_button_is_toggled(GLTK_TOGGLE_BUTTON(priv->shift));
	gboolean isCaps = gltk_toggle_button_is_toggled(GLTK_TOGGLE_BUTTON(priv->caps));

	if (isShift)
	{
		set_shift(keyboard, !isCaps);
	}
	else
	{
		set_shift(keyboard, isCaps);
	}

	gltk_widget_invalidate(GLTK_WIDGET(keyboard));
	return FALSE;
}

static void
add_key_sized(GltkKeyboard* keyboard, GltkBox* box, char key, char shiftKey, float width)
{
	USING_PRIVATE(keyboard);

	KeyboardKey* keyboardKey = g_new(KeyboardKey, 1);
	keyboardKey->key = key;
	keyboardKey->shiftKey = shiftKey;
	keyboardKey->width = width;

	char txt[2] = {key, '\0'};
	keyboardKey->button = gltk_button_new(txt);
	gltk_box_append_widget(box, keyboardKey->button, FALSE, FALSE);
	g_signal_connect(keyboardKey->button, "click-event", (GCallback)key_clicked, keyboard);

	priv->keys = g_slist_prepend(priv->keys, keyboardKey);
}

static void
add_key(GltkKeyboard* keyboard, GltkBox* box, char key, char shiftKey)
{
	add_key_sized(keyboard, box, key, shiftKey, 1.0f);
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
		add_key_sized(self, GLTK_BOX(hboxNumbers), '`', '~', 0.5f);
		add_key(self, GLTK_BOX(hboxNumbers), '1', '!');
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
		GltkWidget* backspace = gltk_button_new("<-");
		gltk_box_append_widget(GLTK_BOX(hboxNumbers), backspace, FALSE, FALSE);
		g_signal_connect(backspace, "click-event", (GCallback)keyBackspace_clicked, self);
		g_signal_connect(backspace, "long-touch-event", (GCallback)keyBackspace_longTouched, self);
	}

	GltkWidget* hboxRow1 = gltk_hbox_new();
	{
		gltk_box_append_widget(GLTK_BOX(hboxRow1), gltk_label_new(" "), FALSE, FALSE);
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
		priv->caps = gltk_toggle_button_new("Caps");
		g_object_ref(priv->caps);
		g_signal_connect_after(priv->caps, "click-event", (GCallback)keyCaps_clicked, self);
		gltk_box_append_widget(GLTK_BOX(hboxRow2), priv->caps, FALSE, FALSE);
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
		priv->shift = gltk_toggle_button_new("Shift");
		g_object_ref(priv->shift);
		g_signal_connect_after(priv->shift, "click-event", (GCallback)keyShift_clicked, self);
		gltk_box_append_widget(GLTK_BOX(hboxRow3), priv->shift, FALSE, FALSE);
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

	GltkWidget* hboxRow4 = gltk_hbox_new();
	{
		gltk_box_append_widget(GLTK_BOX(hboxRow4), gltk_label_new(" "), TRUE, TRUE);
		GltkWidget* space = gltk_button_new("                ");
		gltk_box_append_widget(GLTK_BOX(hboxRow4), space, FALSE, FALSE);
		g_signal_connect(space, "click-event", (GCallback)key_clicked, self);
		gltk_box_append_widget(GLTK_BOX(hboxRow4), gltk_label_new(" "), TRUE, TRUE);
	}

	gltk_box_append_widget(GLTK_BOX(self), hboxTop, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxNumbers, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxRow1, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxRow2, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxRow3, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxRow4, FALSE, FALSE);

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


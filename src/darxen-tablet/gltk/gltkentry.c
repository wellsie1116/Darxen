/* gltkentry.c
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

#include "gltkentry.h"

#include "gltkmarshal.h"
#include "gltkdialog.h"
#include "gltkkeyboard.h"

G_DEFINE_TYPE(GltkEntry, gltk_entry, GLTK_TYPE_BUTTON)

#define USING_PRIVATE(obj) GltkEntryPrivate* priv = GLTK_ENTRY_GET_PRIVATE(obj)
#define GLTK_ENTRY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_ENTRY, GltkEntryPrivate))

enum
{
	TEXT_CHANGED,

	LAST_SIGNAL
};

typedef struct _GltkEntryPrivate		GltkEntryPrivate;
struct _GltkEntryPrivate
{
	int dummy;
};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_entry_dispose(GObject* gobject);
static void gltk_entry_finalize(GObject* gobject);

static void gltk_entry_render(GltkWidget* widget);
static gboolean gltk_entry_click_event(GltkWidget* widget, GltkEventClick* event);

static void
gltk_entry_class_init(GltkEntryClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkEntryPrivate));
	
	signals[TEXT_CHANGED] = 
		g_signal_new(	"text-changed",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkEntryClass, text_changed),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	
	gobject_class->dispose = gltk_entry_dispose;
	gobject_class->finalize = gltk_entry_finalize;

	gltkwidget_class->click_event = gltk_entry_click_event;
	gltkwidget_class->render = gltk_entry_render;

	klass->text_changed = NULL;
}

static void
gltk_entry_init(GltkEntry* self)
{
	//USING_PRIVATE(self);

	/* initialize fields generically here */
}

static void
gltk_entry_dispose(GObject* gobject)
{
	//GltkEntry* self = GLTK_ENTRY(gobject);
	//USING_PRIVATE(self);

	//free and release references

	G_OBJECT_CLASS(gltk_entry_parent_class)->dispose(gobject);
}

static void
gltk_entry_finalize(GObject* gobject)
{
	//GltkEntry* self = GLTK_ENTRY(gobject);
	//USING_PRIVATE(self);

	//free memory

	G_OBJECT_CLASS(gltk_entry_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_entry_new(const char* text)
{
	GObject *gobject = g_object_new(GLTK_TYPE_ENTRY, NULL);
	//GltkEntry* self = GLTK_ENTRY(gobject);

	//USING_PRIVATE(self);
	
	GLTK_BUTTON(gobject)->text = g_strdup(text);

	return (GltkWidget*)gobject;
}

GQuark
gltk_entry_error_quark()
{
	return g_quark_from_static_string("gltk-entry-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_entry_render(GltkWidget* widget)
{
	GLTK_WIDGET_CLASS(gltk_entry_parent_class)->render(widget);
}

static void
dialog_result(GltkDialog* dialog, gboolean success, GltkWidget* entry)
{
	gltk_screen_pop_screen(GLTK_WIDGET(dialog)->screen, GLTK_SCREEN(dialog));

	if (!success)
		return;

	//TODO: compare strings and set our text if necessary

	g_object_ref(entry);
	g_signal_emit(entry, signals[TEXT_CHANGED], 0);
	g_object_unref(entry);
}

static gboolean
gltk_entry_click_event(GltkWidget* widget, GltkEventClick* event)
{
	GltkWidget* keyboard = gltk_keyboard_new();
	GltkDialog* dialog = gltk_dialog_new(keyboard);
	g_signal_connect(dialog, "dialog-result", (GCallback)dialog_result, widget);
	gltk_screen_push_screen(widget->screen, GLTK_SCREEN(dialog));
	return TRUE;
}


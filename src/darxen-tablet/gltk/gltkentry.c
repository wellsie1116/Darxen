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
#include "gltkfonts.h"

#include <GL/gl.h>

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
	GltkDialog* dialog;
	GltkWidget* keyboard;
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
	USING_PRIVATE(self);

	priv->dialog = NULL;
	priv->keyboard = NULL;
}

static void
gltk_entry_dispose(GObject* gobject)
{
	GltkEntry* self = GLTK_ENTRY(gobject);
	USING_PRIVATE(self);

	//free and release references
	if (priv->keyboard)
	{
		g_object_unref(priv->keyboard);
		priv->keyboard = NULL;
	}

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

const gchar*
gltk_entry_get_text(GltkEntry* entry)
{
	return GLTK_BUTTON(entry)->text;
}

GQuark
gltk_entry_error_quark()
{
	return g_quark_from_static_string("gltk-entry-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static float colorBright[] = {0.87f, 0.87f, 0.87f};
static float colorDark[] = {0.78f, 0.78f, 0.78f};

static float colorHighlightBright[] = {1.0f, 0.6f, 0.03f};
static float colorHighlightDark[] = {1.0f, 0.65f, 0.16f};

static void
gltk_entry_render(GltkWidget* widget)
{
	GltkAllocation allocation = gltk_widget_get_allocation(GLTK_WIDGET(widget));

	glBegin(GL_LINE_LOOP);
	{
		float* bright = colorBright;
		float* dark = colorDark;

		if (GLTK_BUTTON(widget)->isDown)
		{
			bright = colorHighlightBright;
			dark = colorHighlightDark;
		}
		glColor3fv(bright);
		glVertex2i(allocation.width, 0);
		glVertex2i(0, 0);
		glColor3fv(dark);
		glVertex2i(0, allocation.height);
		glVertex2i(allocation.width, allocation.height);
	}
	glEnd();

	if (GLTK_BUTTON(widget)->text)
	{
		glPushMatrix();
		{
			GltkGLFont* font = gltk_fonts_cache_get_font(GLTK_FONTS_BASE, 24, TRUE);
			glColor3f(1.0f, 1.0f, 1.0f);

			GltkGLFontBounds bounds = gltk_fonts_measure_string(font, GLTK_BUTTON(widget)->text);

			float x;
			float y;
			x = (allocation.width - bounds.width) / 4.0; //FIXME why not 2?
			y = (allocation.height - bounds.height) / 2.0 + font->ascender + font->descender;

			glTranslatef(x, y, 0.1f);
			glScalef(1.0f, -1.0f, 1.0f);

			ftglRenderFont(font->font, GLTK_BUTTON(widget)->text, FTGL_RENDER_ALL);
		}
		glPopMatrix();
	}
}

static void
dialog_result(GltkDialog* dialog, gboolean success, GltkWidget* entry)
{
	USING_PRIVATE(entry);

	gltk_screen_pop_screen(GLTK_WIDGET(dialog)->screen, GLTK_SCREEN(dialog));

	if (success)
	{
		const gchar* newText = gltk_keyboard_get_text(GLTK_KEYBOARD(priv->keyboard));
		if (g_strcmp0(GLTK_BUTTON(entry)->text, newText))
		{
			g_free(GLTK_BUTTON(entry)->text);
			GLTK_BUTTON(entry)->text = g_strdup(newText);
			gltk_widget_layout(entry);

			g_object_ref(entry);
			g_signal_emit(entry, signals[TEXT_CHANGED], 0);
			g_object_unref(entry);
		}
	}

	g_object_unref(priv->dialog);
	g_object_unref(priv->keyboard);
	priv->dialog = NULL;
	priv->keyboard = NULL;
}

static gboolean
gltk_entry_click_event(GltkWidget* widget, GltkEventClick* event)
{
	USING_PRIVATE(widget);
	priv->keyboard = gltk_keyboard_new(GLTK_BUTTON(widget)->text);
	priv->dialog = gltk_dialog_new(priv->keyboard);
	g_object_ref(priv->dialog);
	g_object_ref(priv->keyboard);
	g_signal_connect(priv->dialog, "dialog-result", (GCallback)dialog_result, widget);
	gltk_screen_push_screen(widget->screen, GLTK_SCREEN(priv->dialog));
	return TRUE;
}


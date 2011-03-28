/* gltktogglebutton.c
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

#include "gltktogglebutton.h"
#include "gltkevents.h"

#include <GL/gl.h>

G_DEFINE_TYPE(GltkToggleButton, gltk_toggle_button, GLTK_TYPE_BUTTON)

#define USING_PRIVATE(obj) GltkToggleButtonPrivate* priv = GLTK_TOGGLE_BUTTON_GET_PRIVATE(obj)
#define GLTK_TOGGLE_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_TOGGLE_BUTTON, GltkToggleButtonPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkToggleButtonPrivate		GltkToggleButtonPrivate;
struct _GltkToggleButtonPrivate
{
	gboolean toggled;

};

//static guint signals[LAST_SIGNAL] = {0,};

static void gltk_toggle_button_dispose(GObject* gobject);
static void gltk_toggle_button_finalize(GObject* gobject);

static gboolean gltk_toggle_button_clicked(GltkWidget* widget, GltkEventClick* event);
static void gltk_toggle_button_render(GltkWidget* widget);

static void
gltk_toggle_button_class_init(GltkToggleButtonClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkToggleButtonPrivate));
	
	gobject_class->dispose = gltk_toggle_button_dispose;
	gobject_class->finalize = gltk_toggle_button_finalize;

	gltkwidget_class->click_event = gltk_toggle_button_clicked;
	gltkwidget_class->render = gltk_toggle_button_render;
}

static void
gltk_toggle_button_init(GltkToggleButton* self)
{
	USING_PRIVATE(self);

	priv->toggled = FALSE;
}

static void
gltk_toggle_button_dispose(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_toggle_button_parent_class)->dispose(gobject);
}

static void
gltk_toggle_button_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_toggle_button_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_toggle_button_new(const char* text)
{
	GObject *gobject = g_object_new(GLTK_TYPE_TOGGLE_BUTTON, NULL);
	GltkToggleButton* self = GLTK_TOGGLE_BUTTON(gobject);

	GLTK_BUTTON(self)->text = g_strdup(text);

	return (GltkWidget*)gobject;
}

gboolean
gltk_toggle_button_is_toggled(GltkToggleButton* button)
{
	g_return_val_if_fail(GLTK_IS_TOGGLE_BUTTON(button), FALSE);
	USING_PRIVATE(button);

	return priv->toggled;
}

void
gltk_toggle_button_set_toggled(GltkToggleButton* button, gboolean value)
{
	g_return_if_fail(GLTK_IS_TOGGLE_BUTTON(button));
	USING_PRIVATE(button);

	priv->toggled = value;
}


GQuark
gltk_toggle_button_error_quark()
{
	return g_quark_from_static_string("gltk-toggle-button-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static gboolean
gltk_toggle_button_clicked(GltkWidget* widget, GltkEventClick* event)
{
	USING_PRIVATE(widget);

	priv->toggled = !priv->toggled;
	gltk_widget_invalidate(widget);

	return FALSE;
}

static void
gltk_toggle_button_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);
	if (priv->toggled)
		glLineWidth(3.0f);
	GLTK_WIDGET_CLASS(gltk_toggle_button_parent_class)->render(widget);
	if (priv->toggled)
		glLineWidth(1.0f);
}


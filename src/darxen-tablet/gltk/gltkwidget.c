/* gltkwidget.c
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

#include "gltkwidget.h"

G_DEFINE_TYPE(GltkWidget, gltk_widget, G_TYPE_OBJECT)

#define USING_PRIVATE(obj) GltkWidgetPrivate* priv = GLTK_WIDGET_GET_PRIVATE(obj)
#define GLTK_WIDGET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_WIDGET, GltkWidgetPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkWidgetPrivate		GltkWidgetPrivate;
struct _GltkWidgetPrivate
{

};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_widget_finalize(GObject* gobject);

static void
gltk_widget_class_init(GltkWidgetClass* klass)
{
	g_type_class_add_private(klass, sizeof(GltkWidgetPrivate));
	
	klass->parent_class.finalize = gltk_widget_finalize;
}

static void
gltk_widget_init(GltkWidget* self)
{
	USING_PRIVATE(self);

	/* initialize fields generically here */
}

static void
gltk_widget_finalize(GObject* gobject)
{
	GltkWidget* self = GLTK_WIDGET(gobject);
	USING_PRIVATE(self);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(gltk_widget_parent_class)->finalize(gobject);

}

GltkWidget*
gltk_widget_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_WIDGET, NULL);
	GltkWidget* self = GLTK_WIDGET(gobject);

	USING_PRIVATE(self);

	return (GltkWidget*)gobject;
}


GQuark
gltk_widget_error_quark()
{
	return g_quark_from_static_string("gltk-widget-error-quark");
}

/*********************
 * Private Functions *
 *********************/


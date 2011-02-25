/* darxensitelist.c
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

#include "darxensitelist.h"

G_DEFINE_TYPE(DarxenSiteList, darxen_site_list, GLTK_TYPE_VBOX)

#define USING_PRIVATE(obj) DarxenSiteListPrivate* priv = DARXEN_SITE_LIST_GET_PRIVATE(obj)
#define DARXEN_SITE_LIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_SITE_LIST, DarxenSiteListPrivate))

enum
{
	//new view selected
	//sites reordered
	//views reordered
	//view deleted
	LAST_SIGNAL
};

typedef struct _DarxenSiteListPrivate		DarxenSiteListPrivate;
struct _DarxenSiteListPrivate
{
	int dummy;

};

//static guint signals[LAST_SIGNAL] = {0,};

static void darxen_site_list_dispose(GObject* gobject);
static void darxen_site_list_finalize(GObject* gobject);

// static void darxen_site_list_size_request(GltkWidget* widget, GltkSize* size);
// static void darxen_site_list_render(GltkWidget* widget);

static void
darxen_site_list_class_init(DarxenSiteListClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(DarxenSiteListPrivate));
	
	gobject_class->dispose = darxen_site_list_dispose;
	gobject_class->finalize = darxen_site_list_finalize;

	// gltkwidget_class->size_request = darxen_site_list_size_request;
	// gltkwidget_class->render = darxen_site_list_render;
}

static void
darxen_site_list_init(DarxenSiteList* self)
{
	USING_PRIVATE(self);

	GltkWidget* btn1 = gltk_button_new("Button 1");
	GltkWidget* btn2 = gltk_button_new("Button 2");
	GltkWidget* btn3 = gltk_button_new("Button 3");

	gltk_box_append_widget(GLTK_BOX(self), btn1, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), btn2, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), btn3, TRUE, FALSE);
}

static void
darxen_site_list_dispose(GObject* gobject)
{
	DarxenSiteList* self = DARXEN_SITE_LIST(gobject);
	USING_PRIVATE(self);

	//free and release references

	G_OBJECT_CLASS(darxen_site_list_parent_class)->dispose(gobject);
}

static void
darxen_site_list_finalize(GObject* gobject)
{
	DarxenSiteList* self = DARXEN_SITE_LIST(gobject);
	USING_PRIVATE(self);

	//free memory

	G_OBJECT_CLASS(darxen_site_list_parent_class)->finalize(gobject);
}

GltkWidget*
darxen_site_list_new()
{
	GObject *gobject = g_object_new(DARXEN_TYPE_SITE_LIST, NULL);
	DarxenSiteList* self = DARXEN_SITE_LIST(gobject);

	USING_PRIVATE(self);

	return (GltkWidget*)gobject;
}


GQuark
darxen_site_list_error_quark()
{
	return g_quark_from_static_string("darxen-site-list-error-quark");
}

/*********************
 * Private Functions *
 *********************/

// static void
// darxen_site_list_size_request(GltkWidget* widget, GltkSize* size)
// {
// 	size->width = 300;
// 	size->height = 400;
// 
// 	GLTK_WIDGET_CLASS(darxen_site_list_parent_class)->size_request(widget, size);
// }
// 
// static void
// darxen_site_list_render(GltkWidget* widget)
// {
// 	glColor3f(0.0, 1.0, 1.0);
// 	glBegin(GL_QUADS);
// 	{
// 		glVertex2i(0, 0);
// 		glVertex2i(0, 100);
// 		glVertex2i(100, 100);
// 		glVertex2i(100, 0);
// 	}
// 	glEnd();
// }


/* gltkminiscreen.c
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

#include "gltkminiscreen.h"

#include <GL/gl.h>

G_DEFINE_TYPE(GltkMiniScreen, gltk_mini_screen, GLTK_TYPE_SCREEN)

#define USING_PRIVATE(obj) GltkMiniScreenPrivate* priv = GLTK_MINI_SCREEN_GET_PRIVATE(obj)
#define GLTK_MINI_SCREEN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_MINI_SCREEN, GltkMiniScreenPrivate))

#define BORDER_WIDTH 20
#define TITLE_HEIGHT 40

enum
{
	LAST_SIGNAL
};

typedef struct _GltkMiniScreenPrivate		GltkMiniScreenPrivate;
struct _GltkMiniScreenPrivate
{
	int dummy;

};

//static guint signals[LAST_SIGNAL] = {0,};
GltkWidget* gltk_screen_get_root(GltkScreen* screen);

static void gltk_mini_screen_dispose(GObject* gobject);
static void gltk_mini_screen_finalize(GObject* gobject);

static void gltk_mini_screen_layout(GltkScreen* screen);
static void gltk_mini_screen_size_request(GltkWidget* widget, GltkSize* size);
static void gltk_mini_screen_render(GltkWidget* widget);

static void
gltk_mini_screen_class_init(GltkMiniScreenClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);
	GltkScreenClass* gltkscreen_class = GLTK_SCREEN_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkMiniScreenPrivate));
	
	gobject_class->dispose = gltk_mini_screen_dispose;
	gobject_class->finalize = gltk_mini_screen_finalize;

	gltkwidget_class->size_request = gltk_mini_screen_size_request;
	gltkwidget_class->render = gltk_mini_screen_render;

	gltkscreen_class->layout = gltk_mini_screen_layout;
}

static void
gltk_mini_screen_init(GltkMiniScreen* self)
{
	GLTK_SCREEN(self)->maximized = FALSE;
}

static void
gltk_mini_screen_dispose(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_mini_screen_parent_class)->dispose(gobject);
}

static void
gltk_mini_screen_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_mini_screen_parent_class)->finalize(gobject);
}

GltkScreen*
gltk_mini_screen_new()
{
	GObject *gobject = g_object_new(GLTK_TYPE_MINI_SCREEN, NULL);

	return (GltkScreen*)gobject;
}


GQuark
gltk_mini_screen_error_quark()
{
	return g_quark_from_static_string("gltk-mini-screen-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_mini_screen_layout(GltkScreen* screen)
{
	GltkWidget* root = gltk_screen_get_root(screen);

	if (!root)
		return;

	GltkAllocation allocation = gltk_widget_get_allocation(GLTK_WIDGET(screen));
	allocation.x += BORDER_WIDTH;
	allocation.y += TITLE_HEIGHT;
	allocation.width -= BORDER_WIDTH * 2;
	allocation.height -= BORDER_WIDTH + TITLE_HEIGHT;

	gltk_widget_size_allocate(root, allocation);
}

static void
gltk_mini_screen_size_request(GltkWidget* widget, GltkSize* size)
{
	GLTK_WIDGET_CLASS(gltk_mini_screen_parent_class)->size_request(widget, size);

	size->width += BORDER_WIDTH * 2;
	size->height += BORDER_WIDTH + TITLE_HEIGHT;

	widget->requisition = *size;
}

static void
gltk_mini_screen_render(GltkWidget* widget)
{
	GltkAllocation allocation = gltk_widget_get_allocation(widget);
	glColor4f(0.1f, 0.1f, 0.1f, 0.9f);
	glRectf(allocation.x, allocation.y, allocation.x + allocation.width, allocation.y + allocation.height);

	int offsetX = BORDER_WIDTH + allocation.x;
	int offsetY = TITLE_HEIGHT + allocation.y;

	glTranslatef(offsetX, offsetY, 0);

	GLTK_WIDGET_CLASS(gltk_mini_screen_parent_class)->render(widget);

	glTranslatef(-offsetX, -offsetY, 0);
}


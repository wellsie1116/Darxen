/* gltkspacer.c
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

#include "gltkspacer.h"

G_DEFINE_TYPE(GltkSpacer, gltk_spacer, GLTK_TYPE_WIDGET)

static void gltk_spacer_render	(GltkWidget* widget);

static void
gltk_spacer_class_init(GltkSpacerClass* klass)
{
	GLTK_WIDGET_CLASS(klass)->render = gltk_spacer_render;
}

static void
gltk_spacer_init(GltkSpacer* self)
{
}

GltkWidget*
gltk_spacer_new(int width, int height)
{
	GObject* gobject = g_object_new(GLTK_TYPE_SPACER, NULL);

	GltkSize size = {width, height};
	gltk_widget_set_size_request(GLTK_WIDGET(gobject), size);

	return (GltkWidget*)gobject;
}

static void
gltk_spacer_render(GltkWidget* widget)
{
}


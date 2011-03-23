/* scrollable.c
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

#include "common.h"

GltkWidget* make_scrollable(GltkWidget* child)
{
	GltkWidget* scrollable = gltk_scrollable_new();

	gltk_scrollable_set_widget(GLTK_SCROLLABLE(scrollable), child);

	return scrollable;
}

GltkWidget* label_widget(GltkWidget* child, const char* message)
{
	GltkWidget* vbox = gltk_vbox_new(0);

	gltk_box_append_widget(GLTK_BOX(vbox), gltk_label_new(message), FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), child, TRUE, TRUE);

	return vbox;
}

GltkWidget* create_large_vbox()
{
	GltkWidget* vbox = gltk_vbox_new(0);

	int i;
	for (i = 0; i < 100; i++)
	{
		gchar* msg = g_strdup_printf("Label %i", i);
		GltkWidget* label = gltk_label_new(msg);
		g_free(msg);

		gltk_box_append_widget(GLTK_BOX(vbox), label, FALSE, FALSE);
	}

	return label_widget(make_scrollable(vbox), "Below are 100 labels");
}

GltkWindow*
create_window()
{
	GltkScreen* screen = gltk_screen_new();
	GltkWindow* window = gltk_window_new();

	GltkWidget* hbox = gltk_hbox_new(0);

	gltk_box_append_widget(GLTK_BOX(hbox), create_large_vbox(), FALSE, FALSE);

	gltk_screen_set_root(screen, hbox);
	gltk_window_push_screen(window, screen);

	return window;
}

/* labels.c
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

GltkWidget* 
create_hbox()
{
	GltkWidget* hbox = gltk_hbox_new();

	GltkWidget* label1 = gltk_label_new("No Expand");
	GltkWidget* label2 = gltk_label_new("Allow Expand, but\ndo not fill.");
	GltkWidget* label3 = gltk_label_new("This label will allow the widget to not only expand\nbut also allow the widget's content to\nexpand to fill available space.");

	gltk_label_set_draw_border(GLTK_LABEL(label1), TRUE);
	gltk_label_set_draw_border(GLTK_LABEL(label2), TRUE);
	gltk_label_set_draw_border(GLTK_LABEL(label3), TRUE);

	gltk_box_append_widget(GLTK_BOX(hbox), label1, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), label2, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), label3, TRUE, TRUE);

	return hbox;
}

GltkWindow*
create_window()
{
	GltkScreen* screen = gltk_screen_new();
	GltkWindow* window = gltk_window_new();

	GltkWidget* vbox = gltk_vbox_new();

	gltk_box_append_widget(GLTK_BOX(vbox), create_hbox(), FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), create_hbox(), TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), create_hbox(), TRUE, TRUE);

	gltk_screen_set_root(screen, vbox);
	gltk_window_push_screen(window, screen);

	return window;
}


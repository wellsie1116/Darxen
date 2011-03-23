/* lists.c
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


//static gboolean
//btnQuit_clicked(GltkWidget* widget, GltkEventClick* event, gpointer user_data)
//{
//	return TRUE;
//}

GltkWidget* label_widget(GltkWidget* child, const char* message)
{
	GltkWidget* vbox = gltk_vbox_new(0);

	gltk_box_append_widget(GLTK_BOX(vbox), gltk_label_new(message), FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), child, TRUE, TRUE);

	return vbox;
}

GltkWidget* 
create_label_list()
{
	GltkWidget* list = gltk_list_new();

	GltkWidget* label1 = gltk_label_new("Item 1");
	GltkWidget* label2 = gltk_label_new("Item 2");
	GltkWidget* label3 = gltk_label_new("Item 3");

	gltk_label_set_draw_border(GLTK_LABEL(label1), TRUE);
	gltk_label_set_draw_border(GLTK_LABEL(label2), TRUE);
	gltk_label_set_draw_border(GLTK_LABEL(label3), TRUE);

	gltk_list_add_item(GLTK_LIST(list), label1, NULL);
	gltk_list_add_item(GLTK_LIST(list), label2, NULL);
	gltk_list_add_item(GLTK_LIST(list), label3, NULL);

	return label_widget(list, "The three labels below are\ncontained in a list so that they\ncan be reordered dynamically.");
}

GltkWidget* 
create_button_list()
{
	GltkWidget* list = gltk_list_new();

	gltk_list_add_item(GLTK_LIST(list), gltk_button_new("Button 1"), NULL);
	gltk_list_add_item(GLTK_LIST(list), gltk_button_new("Button 2"), NULL);
	gltk_list_add_item(GLTK_LIST(list), gltk_button_new("Button 3"), NULL);

	return label_widget(list, "Buttons can be added to\nlists also.");
}

GltkWidget*
create_composite_list_item(const char* caption, const char* btn)
{
	GltkWidget* hbox = gltk_hbox_new(0);

	gltk_box_append_widget(GLTK_BOX(hbox), gltk_label_new(caption), TRUE, TRUE);
	gltk_box_append_widget(GLTK_BOX(hbox), gltk_button_new(btn), TRUE, TRUE);

	return hbox;
}

GltkWidget* 
create_composite_list()
{
	GltkWidget* list = gltk_list_new();

	gltk_list_add_item(GLTK_LIST(list), create_composite_list_item("Item 1", "Btn 1"), NULL);
	gltk_list_add_item(GLTK_LIST(list), create_composite_list_item("Item 2", "Btn 2"), NULL);
	gltk_list_add_item(GLTK_LIST(list), create_composite_list_item("Item 3", "Btn 3"), NULL);

	return label_widget(list, "Any widget can be added to a list.\nThis includes composite widgets and\neven additional lists.");
}

GltkWindow*
create_window()
{
	GltkScreen* screen = gltk_screen_new();
	GltkWindow* window = gltk_window_new();

	GltkWidget* hbox = gltk_hbox_new(0);

	gltk_box_append_widget(GLTK_BOX(hbox), create_label_list(), TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), create_button_list(), TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), create_composite_list(), TRUE, FALSE);

	gltk_screen_set_root(screen, hbox);
	gltk_window_push_screen(window, screen);

	return window;
}


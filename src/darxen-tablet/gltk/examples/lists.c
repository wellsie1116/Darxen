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

GltkWidget* 
create_label_list()
{
	GltkWidget* list = gltk_list_new();

	gltk_list_add_item(GLTK_LIST(list), gltk_label_new("Item 1"), NULL);
	gltk_list_add_item(GLTK_LIST(list), gltk_label_new("Item 2"), NULL);
	gltk_list_add_item(GLTK_LIST(list), gltk_label_new("Item 3"), NULL);

	return list;
}

GltkWidget* 
create_button_list()
{
	GltkWidget* list = gltk_list_new();

	gltk_list_add_item(GLTK_LIST(list), gltk_button_new("Button 1"), NULL);
	gltk_list_add_item(GLTK_LIST(list), gltk_button_new("Button 2"), NULL);
	gltk_list_add_item(GLTK_LIST(list), gltk_button_new("Button 3"), NULL);

	return list;
}

GltkWidget*
create_composite_list_item(const char* caption, const char* btn)
{
	GltkWidget* hbox = gltk_hbox_new();

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

	return list;
}

GltkWindow*
create_window()
{
	GltkWindow* window = gltk_window_new();

	GltkWidget* hbox = gltk_hbox_new();

	gltk_box_append_widget(GLTK_BOX(hbox), create_label_list(), TRUE, TRUE);
	gltk_box_append_widget(GLTK_BOX(hbox), create_button_list(), TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), create_composite_list(), TRUE, FALSE);

	gltk_window_set_root(window, hbox);

	return window;
}


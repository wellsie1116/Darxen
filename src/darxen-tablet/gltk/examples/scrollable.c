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

GltkWidget*
make_scrollable(GltkWidget* child)
{
	GltkWidget* scrollable = gltk_scrollable_new();

	gltk_scrollable_set_widget(GLTK_SCROLLABLE(scrollable), child);

	return scrollable;
}

GltkWidget*
label_widget(GltkWidget* child, const char* message)
{
	GltkWidget* vbox = gltk_vbox_new(0);

	gltk_box_append_widget(GLTK_BOX(vbox), gltk_label_new(message), FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), child, TRUE, TRUE);

	return vbox;
}

GltkWidget*
create_large_vbox()
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

GltkWidget*
create_large_list()
{
	GltkWidget* list = gltk_list_new();

	int i;
	for (i = 0; i < 100; i++)
	{
		gchar* msg = g_strdup_printf("Label %i", i);
		GltkWidget* label = gltk_label_new(msg);
		g_free(msg);

		gltk_list_add_item(GLTK_LIST(list), label, NULL);
	}

	return label_widget(make_scrollable(list), "Below are 100 labels\nin a list");
}

GltkWidget*
create_basic_spinner()
{
	GltkSpinnerModel* model = gltk_spinner_model_new(1);

	gltk_spinner_model_add_toplevel(model, "0", "Item 0");
	gltk_spinner_model_add_toplevel(model, "1", "Item 1");
	gltk_spinner_model_add_toplevel(model, "2", "Item 2");
	gltk_spinner_model_add_toplevel(model, "3", "Item 3");
	gltk_spinner_model_add_toplevel(model, "4", "Item 4");
	gltk_spinner_model_add_toplevel(model, "5", "Item 5");
	gltk_spinner_model_add_toplevel(model, "6", "Item 6");
	gltk_spinner_model_add_toplevel(model, "7", "Item 7");
	gltk_spinner_model_add_toplevel(model, "8", "Item 8");
	gltk_spinner_model_add_toplevel(model, "9", "Item 9");

	GltkWidget* spinner = gltk_spinner_new(model);

	return label_widget(spinner, "Basic Spinner (1 level)");
}

GList*
bilevel_getItems(GltkSpinnerModel* model, int level, int index, gpointer user_data)
{
	GList* res = NULL;

	if (level == 1)
	{
		int i;
		for (i = 0; i < 6; i++)
		{
			gchar* id = g_strdup_printf("%c%i", 'A'+index, i);
			gchar* text = g_strdup_printf("Level %c.%i", 'A'+index, i);

			res = g_list_prepend(res, gltk_spinner_model_item_new(id, text));
			g_free(id);
			g_free(text);
		}
	}
	else
	{
		int i;
		for (i = 0; i < 6; i++)
		{
			gchar* id = g_strdup_printf("%c%i", 'A'+index, i);
			gchar* text = g_strdup_printf("Level %c.%i", 'A'+index, i);

			res = g_list_prepend(res, gltk_spinner_model_item_new(id, text));
			g_free(id);
			g_free(text);
		}
	}

	return g_list_reverse(res);
}

GltkWidget*
create_bilevel_spinner()
{
	GltkSpinnerModel* model = gltk_spinner_model_new(3);

	gltk_spinner_model_add_toplevel(model, "A", "Level A");
	gltk_spinner_model_add_toplevel(model, "B", "Level B");
	gltk_spinner_model_add_toplevel(model, "C", "Level C");
	gltk_spinner_model_add_toplevel(model, "D", "Level D");
	gltk_spinner_model_add_toplevel(model, "E", "Level E");

	g_signal_connect(model, "get-items", (GCallback)bilevel_getItems, NULL);

	GltkWidget* spinner = gltk_spinner_new(model);

	return label_widget(spinner, "Multilevel Spinner (3 levels)");
}

GltkWidget*
create_spinners()
{
	GltkWidget* vboxSpinners = gltk_vbox_new(5);
	{
		gltk_box_append_widget(GLTK_BOX(vboxSpinners), create_basic_spinner(), FALSE, FALSE);
		gltk_box_append_widget(GLTK_BOX(vboxSpinners), create_bilevel_spinner(), FALSE, FALSE);
	}
	return label_widget(vboxSpinners, "Spinners:");
}

GltkWindow*
create_window()
{
	GltkScreen* screen = gltk_screen_new();
	GltkWindow* window = gltk_window_new();

	GltkWidget* hbox = gltk_hbox_new(0);
	{
		gltk_box_append_widget(GLTK_BOX(hbox), create_large_vbox(), FALSE, FALSE);
		gltk_box_append_widget(GLTK_BOX(hbox), create_large_list(), FALSE, FALSE);
		gltk_box_append_widget(GLTK_BOX(hbox), create_spinners(), FALSE, FALSE);
	}

	gltk_screen_set_root(screen, hbox);
	gltk_window_push_screen(window, screen);

	return window;
}

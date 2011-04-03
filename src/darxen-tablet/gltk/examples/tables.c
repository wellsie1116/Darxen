/* tables.c
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
	GltkWidget* hbox = gltk_hbox_new(0);

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

static GltkWidget*
create_table(int padding)
{
	GltkWidget* table = gltk_table_new(8, 8);

	gltk_table_set_row_options(GLTK_TABLE(table), 0, CELL_ALIGN_LEFT, FALSE, padding);
	gltk_table_set_row_options(GLTK_TABLE(table), 1, CELL_ALIGN_LEFT, TRUE, padding);
	gltk_table_set_row_options(GLTK_TABLE(table), 2, CELL_ALIGN_CENTER, FALSE, padding);
	gltk_table_set_row_options(GLTK_TABLE(table), 3, CELL_ALIGN_CENTER, TRUE, padding);
	gltk_table_set_row_options(GLTK_TABLE(table), 4, CELL_ALIGN_RIGHT, FALSE, padding);
	gltk_table_set_row_options(GLTK_TABLE(table), 5, CELL_ALIGN_RIGHT, TRUE, padding);
	gltk_table_set_row_options(GLTK_TABLE(table), 6, CELL_ALIGN_JUSTIFY, FALSE, padding);
	gltk_table_set_row_options(GLTK_TABLE(table), 7, CELL_ALIGN_JUSTIFY, TRUE, padding);

	gltk_table_set_col_options(GLTK_TABLE(table), 0, CELL_ALIGN_LEFT, FALSE, padding);
	gltk_table_set_col_options(GLTK_TABLE(table), 1, CELL_ALIGN_LEFT, TRUE, padding);
	gltk_table_set_col_options(GLTK_TABLE(table), 2, CELL_ALIGN_CENTER, FALSE, padding);
	gltk_table_set_col_options(GLTK_TABLE(table), 3, CELL_ALIGN_CENTER, TRUE, padding);
	gltk_table_set_col_options(GLTK_TABLE(table), 4, CELL_ALIGN_RIGHT, FALSE, padding);
	gltk_table_set_col_options(GLTK_TABLE(table), 5, CELL_ALIGN_RIGHT, TRUE, padding);
	gltk_table_set_col_options(GLTK_TABLE(table), 6, CELL_ALIGN_JUSTIFY, FALSE, padding);
	gltk_table_set_col_options(GLTK_TABLE(table), 7, CELL_ALIGN_JUSTIFY, TRUE, padding);

	int x;
	int y;
	for (x = 0; x < 8; x++)
	{
		for (y = 0; y < 8; y++)
		{
			gchar* msg = g_strdup_printf("label(%i,%i)", x, y);
			GltkWidget* label = gltk_label_new(msg);
			gltk_label_set_draw_border(GLTK_LABEL(label), TRUE);
			gltk_label_set_font_size(GLTK_LABEL(label), y + 8);
			gltk_table_insert_widget(GLTK_TABLE(table), label, x, y);
			g_free(msg);
		}
	}

	return table;
}

GltkWindow*
create_window()
{
	GltkScreen* screen = gltk_screen_new();
	GltkWindow* window = gltk_window_new();

	GltkWidget* scrollable = create_table(5);
//	GltkWidget* scrollable = gltk_scrollable_new();
//	gltk_scrollable_set_widget(GLTK_SCROLLABLE(scrollable), create_table(5));

	gltk_screen_set_root(screen, scrollable);
	gltk_window_push_screen(window, screen);

	return window;
}


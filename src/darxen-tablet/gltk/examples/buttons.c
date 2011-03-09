/* buttons.c
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

static gboolean
button_clicked(GltkWidget* widget, GltkEventClick* event, GltkWidget* label)
{
	gchar* text = g_strdup_printf("Last Button Clicked: %s", gltk_button_get_text(GLTK_BUTTON(widget)));
	gltk_label_set_text(GLTK_LABEL(label), text);
	g_free(text);

	return TRUE;
}

static gboolean
btnQuit_clicked(GltkWidget* widget, GltkEventClick* event, gpointer user_data)
{
	gltk_window_close(widget->window);
	return TRUE;
}

GltkWidget* 
create_vbox()
{
	GltkWidget* vbox = gltk_vbox_new();

	GltkWidget* label = gltk_label_new("Last Button Clicked: None");

	GltkWidget* button1 = gltk_button_new("Button 1");
	GltkWidget* button2 = gltk_button_new("Second Button");
	GltkWidget* button3 = gltk_button_new("Yet Another Button");
	GltkWidget* btnQuit = gltk_button_new("Quit");

	gltk_box_append_widget(GLTK_BOX(vbox), label, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), button1, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), button2, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), button3, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), btnQuit, TRUE, FALSE);

	g_signal_connect(button1, "click-event", G_CALLBACK(button_clicked), label);
	g_signal_connect(button2, "click-event", G_CALLBACK(button_clicked), label);
	g_signal_connect(button3, "click-event", G_CALLBACK(button_clicked), label);
	g_signal_connect(btnQuit, "click-event", G_CALLBACK(btnQuit_clicked), NULL);

	return vbox;
}

GltkWindow*
create_window()
{
	GltkWindow* window = gltk_window_new();

	GltkWidget* hbox = gltk_hbox_new();

	gltk_box_append_widget(GLTK_BOX(hbox), create_vbox(), FALSE, FALSE);

	gltk_window_set_root(window, hbox);

	return window;
}


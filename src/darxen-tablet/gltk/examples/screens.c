/* screens.c
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

static GltkWindow* window;

static gboolean
btn_closeScreen_clicked(GltkWidget* widget, GltkEventClick* event, GltkScreen* screen)
{
	gltk_window_pop_screen(window, screen);

	return TRUE;
}

GltkScreen*
create_second_screen()
{
	GltkScreen* screen = gltk_screen_new();

	GltkWidget* hbox = gltk_hbox_new();

	GltkWidget* label = gltk_label_new("This is a second screen that fills the entire window");

	GltkWidget* vboxEntry = gltk_vbox_new();
	{
		GltkWidget* entry = gltk_entry_new("Change Me");
		
		gltk_box_append_widget(GLTK_BOX(vboxEntry), gltk_label_new("Entry Field:"), FALSE, FALSE);
		gltk_box_append_widget(GLTK_BOX(vboxEntry), entry, FALSE, FALSE);
	}

	GltkWidget* btnCloseSecondScreen = gltk_button_new("Close Screen");

	g_signal_connect(btnCloseSecondScreen, "click-event", (GCallback)btn_closeScreen_clicked, screen);

	gltk_box_append_widget(GLTK_BOX(hbox), label, TRUE, TRUE);
	gltk_box_append_widget(GLTK_BOX(hbox), vboxEntry, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), btnCloseSecondScreen, FALSE, FALSE);

	gltk_screen_set_root(screen, hbox);

	return screen;
}

GltkScreen*
create_mini_screen()
{
	GltkScreen* screen = gltk_mini_screen_new();

	GltkWidget* vbox = gltk_vbox_new();

	GltkWidget* label = gltk_label_new("This is more of\na dialog than a\nscreen");
	GltkWidget* btnCloseMiniScreen = gltk_button_new("Close");

	g_signal_connect(btnCloseMiniScreen, "click-event", (GCallback)btn_closeScreen_clicked, screen);

	gltk_box_append_widget(GLTK_BOX(vbox), label, TRUE, TRUE);
	gltk_box_append_widget(GLTK_BOX(vbox), btnCloseMiniScreen, FALSE, FALSE);

	gltk_screen_set_root(screen, vbox);

	return screen;
}

static gboolean
btnSecondScreen_clicked(GltkWidget* widget, GltkEventClick* event, gpointer user_data)
{
	GltkScreen* secondScreen = create_second_screen();

	gltk_window_push_screen(window, secondScreen);

	return TRUE;
}

static gboolean
btnDialog_clicked(GltkWidget* widget, GltkEventClick* event, gpointer user_data)
{
	GltkScreen* miniScreen = create_mini_screen();

	gltk_window_push_screen(window, miniScreen);

	return TRUE;
}

static gboolean
btnQuit_clicked(GltkWidget* widget, GltkEventClick* event, gpointer user_data)
{
	gltk_screen_close_window(widget->screen);
	return TRUE;
}

GltkWidget* 
create_main_vbox()
{
	GltkWidget* vbox = gltk_vbox_new();

	GltkWidget* label = gltk_label_new("Main Screen, click a button below to show a new screen");

	GltkWidget* btnSecondScreen = gltk_button_new("Second Screen");
	GltkWidget* btnDialog = gltk_button_new("Dialog");
	GltkWidget* btnQuit = gltk_button_new("Quit");

	gltk_box_append_widget(GLTK_BOX(vbox), label, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), btnSecondScreen, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), btnDialog, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), btnQuit, TRUE, FALSE);

	g_signal_connect(btnSecondScreen, "click-event", G_CALLBACK(btnSecondScreen_clicked), NULL);
	g_signal_connect(btnDialog, "click-event", G_CALLBACK(btnDialog_clicked), NULL);
	g_signal_connect(btnQuit, "click-event", G_CALLBACK(btnQuit_clicked), NULL);

	return vbox;
}

GltkScreen*
create_main_screen()
{
	GltkScreen* screen = gltk_screen_new();

	GltkWidget* hbox = gltk_hbox_new();

	gltk_box_append_widget(GLTK_BOX(hbox), create_main_vbox(), FALSE, FALSE);
	
	gltk_screen_set_root(screen, hbox);

	return screen;
}

GltkWindow*
create_window()
{
	window = gltk_window_new();

	GltkScreen* mainScreen = create_main_screen();

	gltk_window_push_screen(window, mainScreen);

	return window;
}


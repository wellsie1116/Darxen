/* darxen-tablet.c
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

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <libgrip/gripgesturemanager.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "darxensitelist.h"
#include "../soap/client/DarxenService.nsmap"

#include <gltk/gltk.h>

static GtkWidget* window;
static GtkWidget* darea;

static GltkWindow* glWindow;

static void
gesture_callback(	GtkWidget* widget,
					GripTimeType time_type,
					GripGestureEvent* event,
					gpointer user_data)
{
	if (time_type == GRIP_TIME_START)
	{
		switch (event->type)
		{
			case GRIP_GESTURE_DRAG:
				g_message("Gesture started: Drag");
			break;
			case GRIP_GESTURE_PINCH:
				g_message("Gesture started: Pinch");
			break;
			case GRIP_GESTURE_ROTATE:
				g_message("Gesture started: Rotate");
			break;
			case GRIP_GESTURE_TAP:
				g_message("Gesture started: Tap");
			break;
		}
	}
	else if (time_type == GRIP_TIME_END)
	{
		g_message("Gesture ended");
	}
	else // (time_type == GRIP_TIME_UPDATE)
	{
	}
}

static gboolean
button_press_event(GtkWidget* widget, GdkEventButton* event)
{
	g_message("Press event");
	GltkEvent* e = gltk_event_new(GLTK_TOUCH);

	e->touch.id = -1;
	e->touch.touchType = TOUCH_BEGIN;
	e->touch.fingers = 1;
	e->touch.positions = g_new(GltkTouchPosition, 1);
	e->touch.positions->x = (int)event->x;
	e->touch.positions->y = (int)event->y;

	gltk_window_send_event(glWindow, e);

	gltk_event_free(e);

	return TRUE;
}

static gboolean
button_release_event(GtkWidget* widget, GdkEventButton* event)
{
	g_message("Release event");
	GltkEvent* e = gltk_event_new(GLTK_TOUCH);

	e->touch.id = -1;
	e->touch.touchType = TOUCH_END;
	e->touch.fingers = 1;
	e->touch.positions = g_new(GltkTouchPosition, 1);
	e->touch.positions->x = (int)event->x;
	e->touch.positions->y = (int)event->y;

	gltk_window_send_event(glWindow, e);

	gltk_event_free(e);

	return TRUE;
}

static gboolean
motion_notify_event(GtkWidget* widget, GdkEventMotion* event)
{
	g_message("Motion event: %lf, %lf", event->x, event->y);

	GltkEvent* e = gltk_event_new(GLTK_TOUCH);

	e->touch.id = -1;
	e->touch.touchType = TOUCH_MOVE;
	e->touch.fingers = 1;
	e->touch.positions = g_new(GltkTouchPosition, 1);
	e->touch.positions->x = (int)event->x;
	e->touch.positions->y = (int)event->y;

	gltk_window_send_event(glWindow, e);

	gltk_event_free(e);

	return TRUE;
}

static gboolean
expose(GtkWidget *darea, GdkEventExpose *event, gpointer user_data)
{
	g_message("Expose");

	GdkGLContext *glcontext = gtk_widget_get_gl_context(darea);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(darea);
	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
	{
		g_error("Failed to draw to GLDrawable");
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// glBegin(GL_TRIANGLES);
	// {
	// 	glColor3f(1.0f, 0.0f, 0.0f);
	// 	glVertex2i(50, 50);
	// 	glColor3f(0.0f, 1.0f, 0.0f);
	// 	glVertex2i(100, 50);
	// 	glColor3f(0.0f, 0.0f, 1.0f);
	// 	glVertex2i(75, 100);
	// }
	// glEnd();

	gltk_window_render(glWindow);

	if (gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	else
		glFlush ();
	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

static gboolean
configure(GtkWidget *darea, GdkEventConfigure *event, gpointer user_data)
{
	g_message("Configure");
	gltk_window_set_size(glWindow, event->width, event->height);

	GdkGLContext *glcontext = gtk_widget_get_gl_context(darea);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(darea);
	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
	{
		g_error("Failed to configure GLDrawable");
	}

	glViewport(0, 0, event->width, event->height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, event->width, event->height, 0.0);
	
	glMatrixMode(GL_MODELVIEW);

	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gltk_window_set_size(glWindow, event->width, event->height);

	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

static void
destroy(GtkWidget* widget, gpointer data)
{
    gtk_main_quit();
}

static void
button_clicked(GtkWidget* widget, gpointer user_data)
{
	const gchar* text = gltk_button_get_text(GLTK_BUTTON(widget));
	printf("Button Clicked: %s\n", text);
}

static void
btnQuit_clicked(GtkWidget* widget, gpointer user_data)
{
	printf("Goodbye\n");
	gtk_main_quit();
}

void
request_render()
{
	gtk_widget_queue_draw(darea);
}

GltkWidget* create_hbox()
{
	GltkWidget* hbox = gltk_hbox_new();

	GltkWidget* label1 = gltk_label_new("label");
	GltkWidget* label2 = gltk_label_new("I am a label");
	GltkWidget* label3 = gltk_label_new("This is a label with a few different\n"
		   								"sentences worth of text.  I am a second\n"
										"sentence. Here is my third sentence of\n"
										"this paragraph; and I can technically be\n"
									    "called the fourth sentence. WOOOOOOOT");

	gltk_box_append_widget(GLTK_BOX(hbox), label1, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), label2, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), label3, TRUE, TRUE);

	return hbox;
}

static gboolean
long_touch(GltkWidget* widget, GltkEventClick* event, gpointer user_data)
{
	if (!g_strcmp0(user_data, "kind"))
	{
		g_message("Long touch intercepted");
		return TRUE;
	}
	else
	{
		g_message("Long touch ignored");
		return FALSE;
	}
}

GltkWidget* create_sublist()
{
	GltkWidget* widget = gltk_list_new();

	GltkWidget* btn1;
	gltk_list_add_item(GLTK_LIST(widget), btn1=gltk_button_new("View Item 1"), NULL);
	gltk_list_add_item(GLTK_LIST(widget), gltk_button_new("View Item 2"), NULL);
	gltk_list_add_item(GLTK_LIST(widget), gltk_button_new("View Item 3"), NULL);

	g_signal_connect(btn1, "click-event", button_clicked, NULL);
	return widget;
}

GltkWidget* create_list_item(const gchar* msg)
{
	GltkWidget* vbox = gltk_vbox_new();
	g_object_force_floating(G_OBJECT(vbox));

	GltkWidget* button = gltk_button_new(msg);
	GltkWidget* hbox = gltk_hbox_new();

	gltk_box_append_widget(GLTK_BOX(hbox), gltk_label_new("  "), FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(hbox), create_sublist(), TRUE, TRUE);

	gltk_box_append_widget(GLTK_BOX(vbox), button, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), hbox, FALSE, FALSE);

	g_signal_connect(button, "long-touch-event", (GCallback)long_touch, (gpointer)msg);

	return vbox;
}

GltkWindow* create_window()
{
	GltkWindowCallbacks callbacks;
	GltkWindow* win;

	callbacks.request_render = request_render;

	win = gltk_window_new(callbacks);

	GltkWidget* hbox = gltk_hbox_new();

	GltkWidget* vbox = gltk_vbox_new();

	gltk_box_append_widget(GLTK_BOX(vbox), create_hbox(), FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), create_hbox(), TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(vbox), create_hbox(), TRUE, TRUE);

	GltkWidget* btns = gltk_hbox_new();
	GltkWidget* button1 = gltk_button_new("btn");
	GltkWidget* button2 = gltk_button_new("Button");
	GltkWidget* button3 = gltk_button_new("This is a button");
	GltkWidget* btnQuit = gltk_button_new("Quit");

	gltk_box_append_widget(GLTK_BOX(btns), button1, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(btns), button2, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(btns), button3, TRUE, FALSE);
	gltk_box_append_widget(GLTK_BOX(btns), btnQuit, TRUE, FALSE);

	g_signal_connect(button1, "click_event", (GCallback)button_clicked, NULL);
	g_signal_connect(button2, "click_event", (GCallback)button_clicked, NULL);
	g_signal_connect(button3, "click_event", (GCallback)button_clicked, NULL);
	g_signal_connect(btnQuit, "click_event", (GCallback)btnQuit_clicked, NULL);

	gltk_box_append_widget(GLTK_BOX(vbox), btns, TRUE, FALSE);

	//GltkWidget* siteList = darxen_site_list_new();
	//darxen_site_list_add_site(DARXEN_SITE_LIST(siteList), "klot");
	//darxen_site_list_add_view(DARXEN_SITE_LIST(siteList), "klot", "Base Reflectivity");
	//darxen_site_list_add_view(DARXEN_SITE_LIST(siteList), "klot", "Radial Velocity");

	//darxen_site_list_add_site(DARXEN_SITE_LIST(siteList), "kind");
	//darxen_site_list_add_view(DARXEN_SITE_LIST(siteList), "kind", "Cached Reflectivity");
	//darxen_site_list_add_view(DARXEN_SITE_LIST(siteList), "kind", "Cool Storm Yesterday");

	//darxen_site_list_add_site(DARXEN_SITE_LIST(siteList), "kilx");
	
	GltkWidget* siteList = gltk_list_new();

	gltk_list_add_item(GLTK_LIST(siteList), create_list_item("klot"), NULL);
	gltk_list_add_item(GLTK_LIST(siteList), create_list_item("kind"), NULL);
	gltk_list_add_item(GLTK_LIST(siteList), create_list_item("kilx"), NULL);
	gltk_list_add_item(GLTK_LIST(siteList), create_list_item("site"), NULL);
	
	GltkScrollable* scrollable = gltk_scrollable_new();
	gltk_scrollable_set_widget(scrollable, siteList);

	gltk_box_append_widget(GLTK_BOX(hbox), scrollable, TRUE, TRUE);
	gltk_box_append_widget(GLTK_BOX(hbox), vbox, TRUE, TRUE);
	
	gltk_window_set_root(win, hbox);

	return win;
}

int main(int argc, char *argv[])
{
	GdkGLConfig* glconfig;
	GripGestureManager* manager;

	gtk_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);

	manager = grip_gesture_manager_get();

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Darxen Tablet Edition");
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

	darea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), darea);
	gtk_widget_add_events(darea, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON1_MOTION_MASK);

	//grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_PINCH,
	//									 2, gesture_callback, NULL, NULL);
	//grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_ROTATE,
	//									 2, gesture_callback, NULL, NULL);
	//grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_DRAG,
	//									 2, gesture_callback, NULL, NULL);
	//grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_TAP,
	//									 2, gesture_callback, NULL, NULL);

	g_signal_connect(darea, "button-press-event", (GCallback)button_press_event, NULL);
	g_signal_connect(darea, "button-release-event", (GCallback)button_release_event, NULL);
	g_signal_connect(darea, "motion-notify-event", (GCallback)motion_notify_event, NULL);

	glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);
	g_assert(glconfig);

	if (!gtk_widget_set_gl_capability(darea, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE))
	{
		g_error("Failed to set GL capability");
	}

	g_signal_connect(darea, "configure-event", G_CALLBACK(configure), NULL);
	g_signal_connect(darea, "expose-event", G_CALLBACK(expose), NULL);

	glWindow = create_window();

	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}


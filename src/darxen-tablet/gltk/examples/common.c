/* common.c
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

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <libgrip/gripgesturemanager.h>

static GtkWidget* window;
static GtkWidget* darea;

static GltkWindow* glWindow;

//BEGIN GTK Events
static void
gesture_callback(	GtkWidget* widget,
					GripTimeType time_type,
					GripGestureEvent* event,
					gpointer user_data)
{
	if (time_type == GRIP_TIME_START)
	{
	//GltkEvent* e = gltk_event_new(GLTK_TOUCH);

	//e->touch.id = -1;
	//e->touch.touchType = TOUCH_BEGIN;
	//e->touch.fingers = 1;
	//e->touch.positions = g_new(GltkTouchPosition, 1);
	//e->touch.positions->x = (int)event->x;
	//e->touch.positions->y = (int)event->y;

	//gltk_window_send_event(glWindow, e);

	//gltk_event_free(e);
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
		switch (event->type)
		{
			case GRIP_GESTURE_DRAG:
				g_message("Gesture ended: Drag");
			break;
			case GRIP_GESTURE_PINCH:
				g_message("Gesture ended: Pinch");
			break;
			case GRIP_GESTURE_ROTATE:
				g_message("Gesture ended: Rotate");
			break;
			case GRIP_GESTURE_TAP:
				g_message("Gesture ended: Tap");
			break;
		}
	}
	else // (time_type == GRIP_TIME_UPDATE)
	{
		switch (event->type)
		{
			case GRIP_GESTURE_DRAG:
				g_message("Gesture move: Drag");
			break;
			case GRIP_GESTURE_PINCH:
			{
				g_message("Gesture move: Pinch");
				GltkEvent* newEvent = gltk_event_new(GLTK_PINCH);
				GripEventGesturePinch* e = (GripEventGesturePinch*)event;

				int offsetX, offsetY;
				gdk_window_get_origin(darea->window, &offsetX, &offsetY);

				newEvent->pinch.dradius = e->radius_delta;
				newEvent->pinch.radius = e->radius;
				newEvent->pinch.center.x = e->focus_x;
				newEvent->pinch.center.y = e->focus_y;

				newEvent->pinch.fingers = e->fingers;
				newEvent->pinch.positions = g_new(GltkTouchPosition, e->fingers);
				int i;
				for (i = 0; i < event->pinch.fingers; i++)
				{
					newEvent->pinch.positions[i].x = e->finger_x[i] - offsetX;
					newEvent->pinch.positions[i].y = e->finger_y[i] - offsetY;
				}

				gltk_window_send_event(glWindow, newEvent);
				gltk_event_free(newEvent);
			} break;
			case GRIP_GESTURE_ROTATE:
				g_message("Gesture move: Rotate");
			break;
			case GRIP_GESTURE_TAP:
				g_message("Gesture move: Tap");
			break;
		}
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
	//g_message("Motion event: %lf, %lf", event->x, event->y);

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
	//g_message("Expose");

	GdkGLContext *glcontext = gtk_widget_get_gl_context(darea);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(darea);
	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
	{
		g_critical("Failed to draw to GLDrawable");
		return FALSE;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

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
	//g_message("Configure");
	gltk_window_set_size(glWindow, event->width, event->height);

	GdkGLContext *glcontext = gtk_widget_get_gl_context(darea);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(darea);
	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
	{
		g_critical("Failed to configure GLDrawable");
		return FALSE;
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
//END GTK Events

static void
request_render(GltkWindow* window, gpointer user_data)
{
	gtk_widget_queue_draw(darea);
}

static void
hide_cursor(GtkWidget* root)
{
	gchar bits[] = { 0 };
	GdkColor color = { 0, 0, 0, 0 };
	GdkPixmap *pixmap;
	GdkCursor *cursor;

	pixmap = gdk_bitmap_create_from_data(NULL, bits, 1, 1);
	cursor = gdk_cursor_new_from_pixmap(pixmap, pixmap, &color, &color, 0, 0);
	gdk_window_set_cursor(root->window, cursor);
}

static void
request_close(GltkWindow* window, gpointer user_data)
{
	gtk_main_quit();
}

int initialize_gui(int* argc, char** argv[])
{
	GdkGLConfig* glconfig;
	GripGestureManager* manager;

	gtk_init(argc, argv);
	gtk_gl_init(argc, argv);

	manager = grip_gesture_manager_get();

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Darxen Tablet Edition");
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

	darea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), darea);
	gtk_widget_add_events(darea, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON1_MOTION_MASK);

	grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_PINCH,
										 2, gesture_callback, NULL, NULL);
	grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_ROTATE,
										 2, gesture_callback, NULL, NULL);
	grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_DRAG,
										 2, gesture_callback, NULL, NULL);
	grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_TAP,
										 2, gesture_callback, NULL, NULL);

	g_signal_connect(darea, "button-press-event", (GCallback)button_press_event, NULL);
	g_signal_connect(darea, "button-release-event", (GCallback)button_release_event, NULL);
	g_signal_connect(darea, "motion-notify-event", (GCallback)motion_notify_event, NULL);

	glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);
	g_assert(glconfig);

	if (!gtk_widget_set_gl_capability(darea, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE))
	{
		g_critical("Failed to set GL capability");
		return 1;
	}

	g_signal_connect(darea, "configure-event", G_CALLBACK(configure), NULL);
	g_signal_connect(darea, "expose-event", G_CALLBACK(expose), NULL);

	glWindow = create_window();
	g_signal_connect(G_OBJECT(glWindow), "request-render", (GCallback)request_render, NULL);
	g_signal_connect(G_OBJECT(glWindow), "close", (GCallback)request_close, NULL);

	gtk_widget_show_all(window);
	
	hide_cursor(darea);

	return 0;
}

int main(int argc, char *argv[])
{
	int res;
	if ((res = initialize_gui(&argc, &argv)))
		return res;

	gtk_main();
	return 0;
}


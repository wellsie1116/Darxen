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
expose(GtkWidget *darea, GdkEventExpose *event, gpointer user_data)
{
	g_message("Expose");

	GdkGLContext *glcontext = gtk_widget_get_gl_context(darea);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(darea);
	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
	{
		g_error("Failed to draw to GLDrawable");
	}

	glClear(GL_COLOR_BUFFER_BIT);
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

	gltk_window_set_size(glWindow, event->width, event->height);

	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

static void
destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void
request_render()
{
	gtk_widget_queue_draw(darea);
}

GltkWindow* create_window()
{
	GltkWindowCallbacks callbacks;
	GltkWindow* win;

	callbacks.request_render = request_render;

	win = gltk_window_new(callbacks);

	GltkWidget* hbox = gltk_hbox_new();

	GltkWidget* label1 = gltk_label_new("label 1");
	GltkWidget* label2 = gltk_label_new("label 2");
	GltkWidget* label3 = gltk_label_new("label 3");

	gltk_hbox_append_widget(GLTK_HBOX(hbox), label1, FALSE, FALSE);
	gltk_hbox_append_widget(GLTK_HBOX(hbox), label2, TRUE, FALSE);
	gltk_hbox_append_widget(GLTK_HBOX(hbox), label3, TRUE, TRUE);

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
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

	darea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), darea);
	gtk_widget_set_events(darea, GDK_EXPOSURE_MASK);

	grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_PINCH,
										 2, gesture_callback, NULL, NULL);
	grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_ROTATE,
										 2, gesture_callback, NULL, NULL);
	grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_DRAG,
										 2, gesture_callback, NULL, NULL);
	grip_gesture_manager_register_window(manager, darea, GRIP_GESTURE_TAP,
										 2, gesture_callback, NULL, NULL);

	glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);
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


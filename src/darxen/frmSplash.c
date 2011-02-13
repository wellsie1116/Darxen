/* frmSplash.c

   Copyright (C) 2011 Kevin Wells <kevin@darxen.org>

   This file is part of darxen.

   darxen is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   darxen is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with darxen.  If not, see <http://www.gnu.org/licenses/>.
*/

#define G_LOG_DOMAIN "DarxenSplash"
#include "glib/gmessages.h"

#include "frmSplash.h"

#include "DarxenRendererShared.h"

#include "libdarxenFileSupport.h"

#include <gtk/gtkglinit.h>
#include <gtk/gtkglwidget.h>
#include <glade/glade-xml.h>

//#ifdef G_OS_WIN32
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#endif
#include <GL/gl.h>
#include <GL/glu.h>

#include "frmMain.h"

typedef enum _ProgressNames		ProgressNames;
typedef struct _Progress			Progress;

enum _ProgressNames
{
	PROGRESS_CHECK_PATHS,
	PROGRESS_LOAD_SETTINGS,
	PROGRESS_LOAD_SHAPEFILES,
	PROGRESS_MAIN_INIT,

	PROGRESS_COUNT
};

struct _Progress
{
	char *chrName;
	GCallback function;
	int intTime;
};

/* Private Functions */
static void		darxen_splash_gl_init();
static gboolean	darxen_splash_load(gpointer data);

static void darxen_splash_check_paths();
//static void darxen_splash_settings_check_version();
static void darxen_splash_settings_get_main();
static void darxen_splash_load_shapefiles();
static void darxen_splash_main_init();

static gboolean	darxen_splash_darea_realize		(GtkWidget *widget);
static gboolean	darxen_splash_darea_configure	(GtkWidget *widget, GdkEventConfigure *event);
static gboolean	darxen_splash_darea_expose		(GtkWidget *widget, GdkEventExpose *event);

static void darxen_splash_darea_draw();

static void		darxen_splash_redraw();

/* Private Variables */
static GtkWidget *mfrmSplash = NULL;
static GtkWidget *darea = NULL;
SettingsGL *glSettings;
static Progress objProgressItems[PROGRESS_COUNT] = {
{"Checking Files",			(GCallback)darxen_splash_check_paths,		1},
{"Loading Settings...",		(GCallback)darxen_splash_settings_get_main,	1},
{"Loading Shapefiles...",	(GCallback)darxen_splash_load_shapefiles,	1},
{"Creating Main Form...",	(GCallback)darxen_splash_main_init,			1}};
static int intProgressTimeCurrent = 0;
static int intProgressTimeTotal = 0;
static int intProgressItemCurrent = 0;

/*********************
 * Private Functions *
 *********************/

static void
darxen_splash_gl_init()
{
	glSettings->glConfig = gdk_gl_config_new_by_mode((	GDK_GL_MODE_RGB |
														GDK_GL_MODE_DEPTH |
														GDK_GL_MODE_DOUBLE |
														GDK_GL_MODE_STENCIL));
	if (!glSettings->glConfig)
		g_error("Error creating OpenGL configuration");
}

static gboolean
darxen_splash_load(gpointer data)
{
	g_debug("starting: %s", objProgressItems[intProgressItemCurrent].chrName);
	(objProgressItems[intProgressItemCurrent].function)();

	intProgressTimeCurrent += objProgressItems[intProgressItemCurrent].intTime;
	intProgressItemCurrent++;
	if (intProgressItemCurrent == PROGRESS_COUNT)
	{
		g_main_loop_quit((GMainLoop*)data);
		g_debug("done: %s", objProgressItems[intProgressItemCurrent-1].chrName);
		return FALSE;
	}

	darxen_splash_redraw();
	g_debug("done: %s", objProgressItems[intProgressItemCurrent-1].chrName);
	return TRUE;
}

static void
darxen_splash_check_paths()
{
	GDir *dir;

	dir = g_dir_open(darxen_file_support_get_app_path(), 0, NULL);
	if (!dir)
		g_mkdir_with_parents(darxen_file_support_get_app_path(), 0777);
	else
		g_dir_close(dir);

	gchar* path;

	path = g_build_filename(darxen_file_support_get_app_path(), "shapefiles", "cache", NULL);
	g_mkdir_with_parents(path, 0777);
	g_free(path);

	path = g_build_filename(darxen_file_support_get_app_path(), "radardata", NULL);
		g_mkdir_with_parents(path, 0777);
		g_free(path);

	settings_check_version();
}

static void darxen_splash_settings_get_main()
{
	SettingsMain *objSettings = settings_get_main();

	if (!objSettings)
		g_error("Settings failed to load");

	GError* error = NULL;

	if (!settings_create_client(&error))
		g_critical(error->message);
}

static void
darxen_splash_load_shapefiles()
{


	//TODO: precache? no
}

static void
darxen_splash_main_init()
{
	GtkWidget *frmMain;

	GtkBuilder* builder;
	GError* error = NULL;

	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "darxen.glade", &error);

	if (error)
	{
		g_error("Failed to load gui: %s", error->message);
	}

	gtk_builder_connect_signals(builder, NULL);

	frmMain = (GtkWidget*)gtk_builder_get_object(builder, "frmMain");
	darxen_main_new(GTK_WINDOW(frmMain), builder);
	g_object_unref(builder);

	gtk_widget_show(frmMain);
}

static gboolean
darxen_splash_darea_realize(GtkWidget *widget)
{
	GdkGLContext *glcontext;
	GdkGLDrawable *gldrawable;

	g_return_val_if_fail(GTK_IS_DRAWING_AREA(widget), TRUE);
	glcontext = gtk_widget_get_gl_context(widget);
	gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
		return TRUE;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glSettings->glShareList = glcontext;
	darxen_renderer_shared_init_text();

	settings_set_gl(glSettings);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	gdk_gl_drawable_gl_end(gldrawable);

	return FALSE;
}

static gboolean
darxen_splash_darea_configure(GtkWidget *widget, GdkEventConfigure *event)
{
	GdkGLContext *glcontext;
	GdkGLDrawable *gldrawable;

	g_return_val_if_fail(GTK_IS_DRAWING_AREA(widget), TRUE);
	glcontext = gtk_widget_get_gl_context(widget);
	gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
		return TRUE;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, event->width, event->height, 0, 1, -1);
	glViewport(0, 0, event->width, event->height);
	glMatrixMode(GL_MODELVIEW);

	gdk_gl_drawable_gl_end(gldrawable);

	return FALSE;
}

static gboolean
darxen_splash_darea_expose(GtkWidget *widget, GdkEventExpose *event)
{
	GdkGLContext *glcontext;
	GdkGLDrawable *gldrawable;

	glcontext = gtk_widget_get_gl_context(widget);
	gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
		return TRUE;

	glClear(GL_COLOR_BUFFER_BIT);


	darxen_splash_darea_draw();


	/* This code was fun, but no
	int i;
	for (i = 190; i > 4; i -= 10)
	{
		int x = g_random_int_range(0, 300 - i);
		int y = g_random_int_range(0, 200 - i);
		glColor4d(g_random_double_range(0.2, 1.0),g_random_double_range(0.2, 1.0),g_random_double_range(0.3, 1.0),g_random_double_range(0.5, 1.0));
		glRecti(x, y, i, i);
	}
	*/

	gdk_gl_drawable_swap_buffers(gldrawable);

	gdk_gl_drawable_gl_end(gldrawable);

	return FALSE;
}

static void darxen_splash_darea_draw()
{
	char chrText[20];

	glBegin(GL_LINE_LOOP);
		glVertex2i(0, 0);
		glVertex2i(300, 0);
		glVertex2i(300, 200);
		glVertex2i(0, 200);
	glEnd();

	sprintf(chrText, "%i/%i", intProgressItemCurrent + 1, PROGRESS_COUNT);
	darxen_renderer_shared_render_text_line(chrText, TEXT_LOWER_LEFT, TRUE, 10, 190);

	darxen_renderer_shared_render_text_line(objProgressItems[intProgressItemCurrent].chrName, TEXT_LOWER_RIGHT, TRUE, 290, 190);
}

static void
darxen_splash_redraw()
{
	gdk_window_invalidate_rect(darea->window, &darea->allocation, FALSE);
}

/********************
 * Public Functions *
 ********************/

int
darxen_splash_run()
{
	GMainLoop *loop;
	int i;

	if (mfrmSplash)
		g_error("Splash screen has been shown already");

	for (i = 0; i < PROGRESS_COUNT; i++)
		intProgressTimeTotal += objProgressItems[i].intTime;

	glSettings = (SettingsGL*)malloc(sizeof(SettingsGL));

	mfrmSplash = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_decorated(GTK_WINDOW(mfrmSplash), FALSE);
	gtk_window_set_title(GTK_WINDOW(mfrmSplash), "loading DARXEN...");
	gtk_window_set_position(GTK_WINDOW(mfrmSplash), GTK_WIN_POS_CENTER);

	darxen_splash_gl_init();
	darea = gtk_drawing_area_new();
	gtk_widget_set_gl_capability(darea, glSettings->glConfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
	g_signal_connect(darea, "realize", (GCallback)darxen_splash_darea_realize, NULL);
	g_signal_connect(darea, "configure_event", (GCallback)darxen_splash_darea_configure, NULL);
	g_signal_connect(darea, "expose_event", (GCallback)darxen_splash_darea_expose, NULL);
	gtk_widget_set_size_request(darea, 300, 200);

	gtk_container_add(GTK_CONTAINER(mfrmSplash), darea);

	gtk_widget_show_all(mfrmSplash);

	loop = g_main_loop_new(NULL, FALSE);

	g_timeout_add_full(G_PRIORITY_LOW, 100, darxen_splash_load, loop, NULL);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	gtk_widget_hide(mfrmSplash);

	return 0;
}

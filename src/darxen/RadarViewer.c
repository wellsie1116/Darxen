/* RadarViewer.c

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

#define G_LOG_DOMAIN "DarxenRadarViewer"
#include "glib/gmessages.h"

#include "RadarViewer.h"

#include "DarxenRenderer.h"

#include <math.h>
#include <stdlib.h>

#include <gtk/gtkmarshal.h>
#include <cairo.h>

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

enum
{
	FRAME_CHANGED,
	LAST_SIGNAL
};

struct _DarxenRadarViewerPrivate
{
	SettingsRadarViewer *objSettings;

	DarxenRenderer *renderer;

	int intTimerID;

	GList *lstFrames;
	gboolean blnFrameDataChanged;
	int intCurrentFrame;
	int intFrameCount;

	float fltMouseXStart;
	float fltMouseYStart;
	int	intButton;
};

/* Events */
static void darxen_radar_viewer_realize				(GtkWidget *widget);
static void darxen_radar_viewer_unrealize			(GtkWidget *widget);
static gboolean darxen_radar_viewer_configure		(GtkWidget *widget, GdkEventConfigure *event);
static gboolean darxen_radar_viewer_expose			(GtkWidget *widget, GdkEventExpose *event);
static gboolean darxen_radar_viewer_motion_notify	(GtkWidget *widget, GdkEventMotion *event);
static gboolean darxen_radar_viewer_button_press	(GtkWidget *widget, GdkEventButton *event);
static gboolean darxen_radar_viewer_button_release	(GtkWidget *widget, GdkEventButton *event);
static gboolean darxen_radar_viewer_scroll			(GtkWidget *widget, GdkEventScroll *event);

static void		darxen_radar_viewer_frame_changed	(DarxenRadarViewer *radview);

/* Private Functions */
void darxen_radar_viewer_invalidate_frame(DarxenRadarViewer *radview);
void darxen_radar_viewer_invalidate_view(DarxenRadarViewer *radview);
static gboolean frame_animate_step(DarxenRadarViewer *radview);

static gint compare_radar_data(ProductsLevel3Data* d1, ProductsLevel3Data* d2);


/* Variables */
static guint radar_viewer_signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE(DarxenRadarViewer, darxen_radar_viewer, GTK_TYPE_DRAWING_AREA)

static void
darxen_radar_viewer_class_init(DarxenRadarViewerClass *class)
{
	GObjectClass *gobject_class;
	GtkWidgetClass *widget_class;
	GtkDrawingAreaClass *darea_class;

	gobject_class = G_OBJECT_CLASS(class);
	widget_class = (GtkWidgetClass*)class;
	darea_class = (GtkDrawingAreaClass*)class;

	widget_class->motion_notify_event	= darxen_radar_viewer_motion_notify;
	widget_class->button_press_event	= darxen_radar_viewer_button_press;
	widget_class->button_release_event	= darxen_radar_viewer_button_release;
	widget_class->scroll_event			= darxen_radar_viewer_scroll;

	class->frame_changed				= darxen_radar_viewer_frame_changed;

	radar_viewer_signals[FRAME_CHANGED] = g_signal_new(
		"frame_changed",
		G_TYPE_FROM_CLASS(gobject_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(DarxenRadarViewerClass, frame_changed),
		NULL, NULL,
		gtk_marshal_VOID__VOID,
		G_TYPE_NONE, 0);
}

static void
darxen_radar_viewer_init(DarxenRadarViewer *radview)
{
	const SettingsGL* objSettingsGL;

	objSettingsGL = settings_get_gl();

	radview->priv = (DarxenRadarViewerPrivate*)malloc(sizeof(DarxenRadarViewerPrivate));

	radview->priv->objSettings			= settings_get_radar_viewer();
	radview->priv->renderer				= NULL;
	radview->priv->lstFrames			= NULL;
	radview->priv->blnFrameDataChanged	= TRUE;
	radview->priv->intCurrentFrame		= 0;
	radview->priv->intFrameCount		= 0;
	radview->priv->intTimerID			= 0;

	if (!gtk_widget_set_gl_capability(GTK_WIDGET(radview), objSettingsGL->glConfig, objSettingsGL->glShareList, TRUE, GDK_GL_RGBA_TYPE))
		g_critical("Failed to set gl capability\n");

	g_signal_connect(G_OBJECT(radview), "realize", G_CALLBACK(darxen_radar_viewer_realize), NULL);
	g_signal_connect(G_OBJECT(radview), "unrealize", G_CALLBACK(darxen_radar_viewer_unrealize), NULL);
	g_signal_connect(G_OBJECT(radview), "configure_event", G_CALLBACK(darxen_radar_viewer_configure), NULL);
	g_signal_connect(G_OBJECT(radview), "expose_event", G_CALLBACK(darxen_radar_viewer_expose), NULL);
}

/********************
 * Public Functions *
 ********************/

GtkWidget*
darxen_radar_viewer_new(SettingsView *viewSettings, SettingsSite *site)
{
	SettingsMain* objSettingsMain;

	GtkWidget* widget = gtk_widget_new(DARXEN_TYPE_RADAR_VIEWER, NULL);
	DarxenRadarViewer* radview = DARXEN_RADAR_VIEWER(widget);

	objSettingsMain = settings_get_main();

	radview->priv->renderer = DARXEN_RENDERER(darxen_renderer_new(viewSettings, site));

	gtk_widget_add_events(widget,	GDK_BUTTON1_MOTION_MASK |
									GDK_BUTTON2_MOTION_MASK |
									GDK_BUTTON3_MOTION_MASK |
									GDK_BUTTON_PRESS_MASK |
									GDK_BUTTON_RELEASE_MASK |
									GDK_SCROLL_MASK);

	return widget;
}

/* Get */
gboolean
darxen_radar_viewer_get_smoothing(DarxenRadarViewer *radview)
{
	if (!radview)
		g_return_val_if_reached(FALSE);

	return settings_get_main()->renderer.blnSmoothing;
}

gboolean
darxen_radar_viewer_get_frame_animate(DarxenRadarViewer *radview)
{
	if (!radview)
		g_return_val_if_reached(FALSE);

	return (radview->priv->intTimerID != 0);
}

int
darxen_radar_viewer_get_frame_count(DarxenRadarViewer *radview)
{
	if (!radview)
		g_return_val_if_reached(-1);

	return radview->priv->intFrameCount;
}

int
darxen_radar_viewer_get_frame_current(DarxenRadarViewer *radview)
{
	if (!radview)
		g_return_val_if_reached(-1);

	return radview->priv->intCurrentFrame;
}

ProductsLevel3Data*
darxen_radar_viewer_get_current_frame_data(DarxenRadarViewer *radview)
{
	if (!radview)
		g_return_val_if_reached(NULL);

	return (ProductsLevel3Data*)radview->priv->lstFrames->data;
}


/* Set */
void
darxen_radar_viewer_set_smoothing(DarxenRadarViewer *radview, gboolean smooth)
{
	if (!radview)
		g_return_if_reached();

	if (smooth != settings_get_main()->renderer.blnSmoothing)
	{
		settings_get_main()->renderer.blnSmoothing = smooth;
		darxen_radar_viewer_invalidate_frame(radview);
	}
}

void
darxen_radar_viewer_set_frame_animate(DarxenRadarViewer *radview, gboolean animate)
{
	if (!radview)
		g_return_if_reached();

	if (radview->priv->intTimerID == 0 && animate)
	{
		radview->priv->intTimerID = gtk_timeout_add(radview->priv->objSettings->intFrameStepInterval, (GtkFunction)frame_animate_step, &radview->darea.widget);
		frame_animate_step(radview);
	}
	else if (radview->priv->intTimerID != 0 && !animate)
	{
		gtk_timeout_remove(radview->priv->intTimerID);
		radview->priv->intTimerID = 0;
	}
}

void
darxen_radar_viewer_set_frame_first(DarxenRadarViewer *radview)
{
	if (!radview)
		g_return_if_reached();

	radview->priv->intCurrentFrame = 1;
	radview->priv->lstFrames = g_list_first(radview->priv->lstFrames);
	darxen_radar_viewer_invalidate_frame(radview);
	g_signal_emit(radview, radar_viewer_signals[FRAME_CHANGED], 0);
}

void
darxen_radar_viewer_set_frame_last(DarxenRadarViewer *radview)
{
	if (!radview)
		g_return_if_reached();

	radview->priv->intCurrentFrame = darxen_radar_viewer_get_frame_count(radview);
	radview->priv->lstFrames = g_list_last(radview->priv->lstFrames);
	darxen_radar_viewer_invalidate_frame(radview);
	g_signal_emit(radview, radar_viewer_signals[FRAME_CHANGED], 0);
}

void
darxen_radar_viewer_set_frame_next(DarxenRadarViewer *radview)
{
	if (!radview)
		g_return_if_reached();

	if (radview->priv->intCurrentFrame < darxen_radar_viewer_get_frame_count(radview))
	{
		radview->priv->intCurrentFrame++;
		radview->priv->lstFrames = g_list_next(radview->priv->lstFrames);
		darxen_radar_viewer_invalidate_frame(radview);
		g_signal_emit(radview, radar_viewer_signals[FRAME_CHANGED], 0);
	}
}

void
darxen_radar_viewer_set_frame_previous(DarxenRadarViewer *radview)
{
	if (!radview)
		g_return_if_reached();

	if (radview->priv->intCurrentFrame > 1)
	{
		radview->priv->intCurrentFrame--;
		radview->priv->lstFrames = g_list_previous(radview->priv->lstFrames);
		darxen_radar_viewer_invalidate_frame(radview);
		g_signal_emit(radview, radar_viewer_signals[FRAME_CHANGED], 0);
	}
}

/* utility functions */

void darxen_radar_viewer_toggle_frame_animate(DarxenRadarViewer *radview)
{
	if (!radview)
		g_return_if_reached();

	darxen_radar_viewer_set_frame_animate(radview, !darxen_radar_viewer_get_frame_animate(radview));
}

void
darxen_radar_viewer_append_frame(DarxenRadarViewer *radview, ProductsLevel3Data *objData)
{
	if (!radview)
		g_return_if_reached();

	if (radview->priv->lstFrames == NULL)
	{
		radview->priv->intCurrentFrame = 1;
	}

	radview->priv->lstFrames = g_list_insert_sorted(radview->priv->lstFrames, objData, (GCompareFunc)compare_radar_data);

	radview->priv->intFrameCount++;
}

/**********
 * Events *
 **********/

static void
darxen_radar_viewer_realize(GtkWidget *widget)
{
	DarxenRadarViewer *radview;
	GdkGLContext *glcontext;
	GdkGLDrawable *gldrawable;

	printf("Realize\n");fflush(stdout);

	g_return_if_fail(DARXEN_IS_RADAR_VIEWER(widget));
	radview = DARXEN_RADAR_VIEWER(widget);
	glcontext = gtk_widget_get_gl_context(widget);
	gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	gdk_gl_drawable_gl_end (gldrawable);

}

static void
darxen_radar_viewer_unrealize(GtkWidget *widget)
{
	DarxenRadarViewer *radview;

	printf("Unrealize\n");fflush(stdout);

	g_return_if_fail(DARXEN_IS_RADAR_VIEWER(widget));
	radview = DARXEN_RADAR_VIEWER(widget);
}

static gboolean
darxen_radar_viewer_configure(GtkWidget *widget, GdkEventConfigure *event)
{
	DarxenRadarViewer *radview;
	GdkGLContext *glcontext;
	GdkGLDrawable *gldrawable;

	printf("Configure: %ix%i\n", event->width, event->height);fflush(stdout);

	g_return_val_if_fail(DARXEN_IS_RADAR_VIEWER(widget), TRUE);
	radview = DARXEN_RADAR_VIEWER(widget);
	glcontext = gtk_widget_get_gl_context(widget);
	gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
	{
		g_warning("Failed to begin visual in configure");fflush(stdout);
		return TRUE;
	}

	glViewport(0, 0, event->width, event->height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (event->height > event->width)
	{
		double aspect = (double)event->height / event->width;
		glOrtho(-1, 1, -aspect, aspect, -1, 1);
	}
	else
	{
		double aspect = (double)event->width / event->height;
		glOrtho(-aspect, aspect, -1, 1, -1, 1);
	}
	glMatrixMode(GL_MODELVIEW);

	darxen_renderer_set_size(radview->priv->renderer, event->width, event->height);

	gdk_gl_drawable_gl_end(gldrawable);

	return FALSE;
}

static gboolean
darxen_radar_viewer_expose(GtkWidget *widget, GdkEventExpose *event)
{
	DarxenRadarViewer *radview;
	GdkGLContext *glcontext;
	GdkGLDrawable *gldrawable;
	SettingsRadarViewer *objSettings;
//	int intSize;
//	int width;
//	int height;

	printf("Expose\n");fflush(stdout);

	g_return_val_if_fail(DARXEN_IS_RADAR_VIEWER(widget), TRUE);
	radview = DARXEN_RADAR_VIEWER(widget);
	glcontext = gtk_widget_get_gl_context(widget);
	gldrawable = gtk_widget_get_gl_drawable(widget);
	objSettings = radview->priv->objSettings;
//	width = widget->allocation.width;
//	height = widget->allocation.height;
//	intSize = MAX(width, height);

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return TRUE;

	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();

	if (radview->priv->lstFrames)
	{

		if (radview->priv->blnFrameDataChanged)
		{
			darxen_renderer_set_data(radview->priv->renderer, (ProductsLevel3Data*)(radview->priv->lstFrames->data));
			radview->priv->blnFrameDataChanged = FALSE;
		}

//		glEnable(GL_STENCIL_TEST);
//
//		glClear(GL_STENCIL_BUFFER_BIT);
//		glStencilFunc(GL_ALWAYS, 0x1, 0x1);


//		glBegin(GL_QUADS);
//		{
//			ProductsLevel3Data* data = darxen_renderer_get_data(radview->priv->renderer);
//			float latCenter = data->objDescription.fltLat;
//			float lonCenter = data->objDescription.fltLon;
//#define VIEW_AREA 5.0f
//			glVertex2fv(darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, latCenter + VIEW_AREA, lonCenter + VIEW_AREA));
//			glVertex2fv(darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, latCenter - VIEW_AREA, lonCenter + VIEW_AREA));
//			glVertex2fv(darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, latCenter - VIEW_AREA, lonCenter - VIEW_AREA));
//			glVertex2fv(darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, latCenter + VIEW_AREA, lonCenter - VIEW_AREA));
//		}
//		glEnd();

//		glStencilFunc(GL_EQUAL, 0x0, 0x1);
		darxen_renderer_render(radview->priv->renderer);

//		glDisable(GL_STENCIL_TEST);

	}

	gdk_gl_drawable_swap_buffers(gldrawable);
	gdk_gl_drawable_gl_end(gldrawable);

	return FALSE;
}

static gboolean
darxen_radar_viewer_motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
	DarxenRadarViewer *radview;
	SettingsRenderer *renderSettings;
	int intSize = MAX(widget->allocation.width, widget->allocation.height);

	g_return_val_if_fail(DARXEN_IS_RADAR_VIEWER(widget), TRUE);

	radview = DARXEN_RADAR_VIEWER(widget);
	renderSettings = &settings_get_main()->renderer;

	if (radview->priv->intButton == 1)
	{
		renderSettings->fltOffsetX -= (radview->priv->fltMouseXStart - (float)event->x / intSize) / (renderSettings->fltScale / 2.0f);
		renderSettings->fltOffsetY += (radview->priv->fltMouseYStart - (float)event->y / intSize) / (renderSettings->fltScale / 2.0f);
		radview->priv->fltMouseXStart = (float)(event->x / intSize);
		radview->priv->fltMouseYStart = (float)(event->y / intSize);
		if (renderSettings->fltOffsetX < -OFFSET_MAX)
			renderSettings->fltOffsetX = -OFFSET_MAX;
		else if (renderSettings->fltOffsetX > OFFSET_MAX)
			renderSettings->fltOffsetX = OFFSET_MAX;
		if (renderSettings->fltOffsetY < -OFFSET_MAX)
			renderSettings->fltOffsetY = -OFFSET_MAX;
		else if (renderSettings->fltOffsetY > OFFSET_MAX)
			renderSettings->fltOffsetY = OFFSET_MAX;
		darxen_radar_viewer_invalidate_view(radview);
	}
	else if (radview->priv->intButton == 3)
	{
		int offset = (int)(event->y - radview->priv->fltMouseYStart * intSize);

		if (offset < 0)
		{
			if (settings_get_main()->renderer.fltScale == ZOOM_MAX)
				return FALSE;
			renderSettings->fltScale += ZOOM_STEP * renderSettings->fltScale * 0.25f;
		}
		else if (offset > 0)
		{
			if (renderSettings->fltScale == ZOOM_MIN)
				return FALSE;
			renderSettings->fltScale -= ZOOM_STEP * renderSettings->fltScale * 0.25f;
		}
		else
			return FALSE;

		radview->priv->fltMouseYStart = (float)(event->y / intSize);

		if (renderSettings->fltScale < ZOOM_MIN)
			renderSettings->fltScale = ZOOM_MIN;
		else if (renderSettings->fltScale > ZOOM_MAX)
			renderSettings->fltScale = ZOOM_MAX;

		darxen_radar_viewer_invalidate_view(radview);
	}
	return FALSE;
}

static gboolean
darxen_radar_viewer_button_press(GtkWidget *widget, GdkEventButton *event)
{
	DarxenRadarViewer *radview;
	int intSize = MAX(widget->allocation.width, widget->allocation.height);

	g_return_val_if_fail(DARXEN_IS_RADAR_VIEWER(widget), TRUE);
	radview = DARXEN_RADAR_VIEWER(widget);

	radview->priv->fltMouseXStart = (float)(event->x / intSize);
	radview->priv->fltMouseYStart = (float)(event->y / intSize);
	radview->priv->intButton = event->button;

	return FALSE;
}

static gboolean
darxen_radar_viewer_button_release(GtkWidget *widget, GdkEventButton *event)
{
	g_return_val_if_fail(DARXEN_IS_RADAR_VIEWER(widget), TRUE);

	return FALSE;
}

static gboolean
darxen_radar_viewer_scroll (GtkWidget *widget, GdkEventScroll *event)
{
	DarxenRadarViewer *radview;
	SettingsRenderer *renderSettings;

	g_return_val_if_fail(DARXEN_IS_RADAR_VIEWER(widget), TRUE);

	radview = DARXEN_RADAR_VIEWER(widget);
	renderSettings = &settings_get_main()->renderer;

	/*
	TODO: manip the offset so it appears to zoom from the cursor

	int intSize = MAX(get_width(), get_height());

	mfltOffsetX += (event->x / (intSize / 2) - 1.0) / (mfltScale / 2);
	mfltOffsetY += (event->y / (intSize / 2) - 1.0) / (mfltScale / 2);

	mfltOffsetX += (intSize - event->x - (intSize / 2)) / intSize / mfltScale;
	mfltOffsetY += (intSize - event->y - (intSize / 2)) / intSize / mfltScale;

	mfltOffsetX -= (event->x - (intSize / 2) - 1) / intSize / (mfltScale);
	mfltOffsetX -= (event->y - (intSize / 2) - 1) / intSize / (mfltScale);
	*/

	if (event->direction == GDK_SCROLL_UP)
	{
		if (settings_get_main()->renderer.fltScale == ZOOM_MAX)
			return FALSE;
		renderSettings->fltScale += ZOOM_STEP * renderSettings->fltScale;
	}
	else if (event->direction == GDK_SCROLL_DOWN)
	{
		if (renderSettings->fltScale == ZOOM_MIN)
			return FALSE;
		renderSettings->fltScale -= ZOOM_STEP * renderSettings->fltScale;
	}
	else
		return FALSE;

	if (renderSettings->fltScale < ZOOM_MIN)
		renderSettings->fltScale = ZOOM_MIN;
	else if (renderSettings->fltScale > ZOOM_MAX)
		renderSettings->fltScale = ZOOM_MAX;

//	g_debug("Scale: %f", renderSettings->fltScale);

	darxen_radar_viewer_invalidate_view(radview);

	return FALSE;
}

static void
darxen_radar_viewer_frame_changed(DarxenRadarViewer *radview)
{
}

/*********************
 * Private Functions *
 *********************/

void
darxen_radar_viewer_invalidate_frame(DarxenRadarViewer *radview)
{
	radview->priv->blnFrameDataChanged = TRUE;
	darxen_radar_viewer_invalidate_view(radview);
}

void
darxen_radar_viewer_invalidate_view(DarxenRadarViewer *radview)
{
	gdk_window_invalidate_rect(radview->darea.widget.window, &radview->darea.widget.allocation, FALSE);
}

static gboolean
frame_animate_step(DarxenRadarViewer *radview)
{
	if (g_list_next(radview->priv->lstFrames) == NULL)
		darxen_radar_viewer_set_frame_first(radview);
	else
		darxen_radar_viewer_set_frame_next(radview);
	return TRUE;
}

static gint
compare_radar_data(ProductsLevel3Data* d1, ProductsLevel3Data* d2)
{
	int dateres = g_date_compare(d1->objHeader.objDate, d2->objHeader.objDate);

	if (dateres)
		return dateres;

	if (d1->objHeader.intTime < d2->objHeader.intTime)
		return -1;
	return (d1->objHeader.intTime > d2->objHeader.intTime);
}

/* darxenview.c
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

#include "darxenview.h"

#include "darxenconfig.h"
#include "darxenradarviewer.h"

G_DEFINE_TYPE(DarxenView, darxen_view, GLTK_TYPE_VBOX)

#define USING_PRIVATE(obj) DarxenViewPrivate* priv = DARXEN_VIEW_GET_PRIVATE(obj)
#define DARXEN_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_VIEW, DarxenViewPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _DarxenViewPrivate		DarxenViewPrivate;
struct _DarxenViewPrivate
{
	gchar* site;
	DarxenViewInfo* viewInfo;
	GltkWidget* label;
	GltkWidget* btnPlay;

	guint animateTimeout;

	DarxenRadarViewer* radarViewer;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void darxen_view_dispose(GObject* gobject);
static void darxen_view_finalize(GObject* gobject);

static void darxen_view_set_screen(GltkWidget* widget, GltkScreen* screen);

static void		btnFirst_clicked		(GltkButton* button, GltkEventClick* event, DarxenView* view);
static void		btnPrevious_clicked		(GltkButton* button, GltkEventClick* event, DarxenView* view);
static void		btnPlay_clicked			(GltkButton* button, GltkEventClick* event, DarxenView* view);
static void		btnNext_clicked			(GltkButton* button, GltkEventClick* event, DarxenView* view);
static void		btnLast_clicked			(GltkButton* button, GltkEventClick* event, DarxenView* view);

static void		config_viewNameChanged	(	DarxenConfig* config,
											const gchar* site,
											DarxenViewInfo* viewInfo,
											const gchar* oldName,
											DarxenView* view);

static void		config_viewUpdated		(	DarxenConfig* config,
											const gchar* site,
											DarxenViewInfo* viewInfo,
											DarxenView* view);

static void
darxen_view_class_init(DarxenViewClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(DarxenViewPrivate));
	
	gobject_class->dispose = darxen_view_dispose;
	gobject_class->finalize = darxen_view_finalize;

	gltkwidget_class->set_screen = darxen_view_set_screen;
}

static void
darxen_view_init(DarxenView* self)
{
	USING_PRIVATE(self);

	priv->site = NULL;
	priv->viewInfo = NULL;
	priv->label = NULL;
	priv->btnPlay = NULL;
	priv->radarViewer = NULL;
}

static void
darxen_view_dispose(GObject* gobject)
{
	DarxenView* self = DARXEN_VIEW(gobject);
	USING_PRIVATE(self);

	if (priv->label)
	{
		g_object_unref(priv->label);
		priv->label = NULL;
	}
	
	if (priv->btnPlay)
	{
		g_object_unref(priv->btnPlay);
		priv->btnPlay = NULL;
	}
	
	if (priv->radarViewer)
	{
		g_object_unref(priv->radarViewer);
		priv->radarViewer = NULL;
	}

	G_OBJECT_CLASS(darxen_view_parent_class)->dispose(gobject);
}

static void
darxen_view_finalize(GObject* gobject)
{
	DarxenView* self = DARXEN_VIEW(gobject);
	USING_PRIVATE(self);

	g_free(priv->site);

	G_OBJECT_CLASS(darxen_view_parent_class)->finalize(gobject);
}

GltkWidget*
darxen_view_new(const gchar* site, DarxenViewInfo* viewInfo)
{
	GObject *gobject = g_object_new(DARXEN_TYPE_VIEW, NULL);
	DarxenView* self = DARXEN_VIEW(gobject);
	DarxenConfig* config = darxen_config_get_instance();

	USING_PRIVATE(self);

	priv->site = g_strdup(site);
	priv->viewInfo = viewInfo;

	GltkWidget* hboxHeader = gltk_hbox_new(0);
	{
		gchar* desc = g_strdup_printf("View entited: %s", viewInfo->name);
		priv->label = gltk_label_new(desc);
		g_object_ref(priv->label);
		gltk_box_append_widget(GLTK_BOX(hboxHeader), priv->label, TRUE, TRUE);
		g_free(desc);

		GltkWidget* hboxAnimation = gltk_hbox_new(3);
		{
			GltkWidget* btnFirst = gltk_button_new("|<");
			GLTK_BUTTON(btnFirst)->renderStyle = GLTK_BUTTON_RENDER_OUTLINE;
			g_signal_connect(G_OBJECT(btnFirst), "click-event", (GCallback)btnFirst_clicked, self);
			gltk_box_append_widget(GLTK_BOX(hboxAnimation), btnFirst, FALSE, FALSE);
			
			GltkWidget* btnPrevious = gltk_button_new("<<");
			GLTK_BUTTON(btnPrevious)->renderStyle = GLTK_BUTTON_RENDER_OUTLINE;
			g_signal_connect(G_OBJECT(btnPrevious), "click-event", (GCallback)btnPrevious_clicked, self);
			gltk_box_append_widget(GLTK_BOX(hboxAnimation), btnPrevious, FALSE, FALSE);
			
			priv->btnPlay = gltk_button_new(">");
			g_object_ref(priv->btnPlay);
			GLTK_BUTTON(priv->btnPlay)->renderStyle = GLTK_BUTTON_RENDER_OUTLINE;
			g_signal_connect(G_OBJECT(priv->btnPlay), "click-event", (GCallback)btnPlay_clicked, self);
			gltk_box_append_widget(GLTK_BOX(hboxAnimation), priv->btnPlay, FALSE, FALSE);
			
			GltkWidget* btnNext = gltk_button_new(">>");
			GLTK_BUTTON(btnNext)->renderStyle = GLTK_BUTTON_RENDER_OUTLINE;
			g_signal_connect(G_OBJECT(btnNext), "click-event", (GCallback)btnNext_clicked, self);
			gltk_box_append_widget(GLTK_BOX(hboxAnimation), btnNext, FALSE, FALSE);
			
			GltkWidget* btnLast = gltk_button_new(">|");
			GLTK_BUTTON(btnLast)->renderStyle = GLTK_BUTTON_RENDER_OUTLINE;
			g_signal_connect(G_OBJECT(btnLast), "click-event", (GCallback)btnLast_clicked, self);
			gltk_box_append_widget(GLTK_BOX(hboxAnimation), btnLast, FALSE, FALSE);
		}
		gltk_box_append_widget(GLTK_BOX(hboxHeader), hboxAnimation, FALSE, FALSE);
	}
	gltk_box_append_widget(GLTK_BOX(self), hboxHeader, FALSE, FALSE);

	priv->radarViewer = darxen_radar_viewer_new(site, viewInfo);
	g_object_ref(G_OBJECT(priv->radarViewer));
	gltk_box_append_widget(GLTK_BOX(self), GLTK_WIDGET(priv->radarViewer), TRUE, TRUE);
	
	g_signal_connect(config, "view-name-changed", (GCallback)config_viewNameChanged, self);
	g_signal_connect(config, "view-updated", (GCallback)config_viewUpdated, self);

	return (GltkWidget*)gobject;
}


GQuark
darxen_view_error_quark()
{
	return g_quark_from_static_string("darxen-view-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static gboolean
animate_step(DarxenView* view)
{
	USING_PRIVATE(view);

	if (darxen_radar_viewer_has_frame_next(priv->radarViewer))
		darxen_radar_viewer_frame_next(priv->radarViewer);
	else
		darxen_radar_viewer_frame_first(priv->radarViewer);

	return TRUE;
}

static void
btnFirst_clicked(GltkButton* button, GltkEventClick* event, DarxenView* view)
{
	USING_PRIVATE(view);

	darxen_radar_viewer_frame_first(priv->radarViewer);
}

static void
btnPrevious_clicked(GltkButton* button, GltkEventClick* event, DarxenView* view)
{
	USING_PRIVATE(view);

	darxen_radar_viewer_frame_prev(priv->radarViewer);
}

static void
btnPlay_clicked(GltkButton* button, GltkEventClick* event, DarxenView* view)
{
	USING_PRIVATE(view);

	if (priv->animateTimeout)
	{
		g_source_remove(priv->animateTimeout);
		priv->animateTimeout = 0;
		gltk_button_set_text(GLTK_BUTTON(priv->btnPlay), ">");
	}
	else
	{
		GltkSize size;
		gltk_widget_size_request(priv->btnPlay, &size);
		gltk_widget_set_size_request(priv->btnPlay, size);

		priv->animateTimeout = g_timeout_add(250, (GSourceFunc)animate_step, view);
		gltk_button_set_text(GLTK_BUTTON(priv->btnPlay), "||");
	}
}

static void
btnNext_clicked(GltkButton* button, GltkEventClick* event, DarxenView* view)
{
	USING_PRIVATE(view);

	darxen_radar_viewer_frame_next(priv->radarViewer);
}

static void
btnLast_clicked(GltkButton* button, GltkEventClick* event, DarxenView* view)
{
	USING_PRIVATE(view);

	darxen_radar_viewer_frame_last(priv->radarViewer);
}


static void
config_viewNameChanged(	DarxenConfig* config,
						const gchar* site,
						DarxenViewInfo* viewInfo,
						const gchar* oldName,
						DarxenView* view)
{
	USING_PRIVATE(view);

	if (priv->viewInfo != viewInfo)
		return;

	gchar* desc = g_strdup_printf("View entited: %s", viewInfo->name);
	gltk_label_set_text(GLTK_LABEL(priv->label), desc);
	g_free(desc);
}

static void		
config_viewUpdated(	DarxenConfig* config,
					const gchar* site,
					DarxenViewInfo* viewInfo,
					DarxenView* view)
{
}

static void
darxen_view_set_screen(GltkWidget* widget, GltkScreen* screen)
{
	USING_PRIVATE(widget);

	if (priv->animateTimeout && !screen)
	{
		g_source_remove(priv->animateTimeout);
		priv->animateTimeout = 0;
		gltk_button_set_text(GLTK_BUTTON(priv->btnPlay), ">");
	}

	GLTK_WIDGET_CLASS(darxen_view_parent_class)->set_screen(widget, screen);
}


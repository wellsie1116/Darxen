/* RadarViewer.h

   Copyright (C) 2008 Kevin Wells <kevin@darxen.org>

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

#ifndef __RADARVIEWER_H__
#define __RADARVIEWER_H__

#define ZOOM_MIN 0.0003f
#define ZOOM_MAX 3000.0f
#define ZOOM_STEP 0.3f
#define OFFSET_MAX 600.0f

#include "DarxenCommon.h"
#include "DarxenSettings.h"
#include "DarxenDataStructs.h"

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkdrawingarea.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkglwidget.h>

G_BEGIN_DECLS

#define DARXEN_TYPE_RADAR_VIEWER			(darxen_radar_viewer_get_type())
#define DARXEN_RADAR_VIEWER(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_RADAR_VIEWER, DarxenRadarViewer))
#define DARXEN_RADAR_VIEWER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_RADAR_VIEWER, DarxenRadarViewerClass))
#define DARXEN_IS_RADAR_VIEWER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_RADAR_VIEWER))
#define DARXEN_IS_RADAR_VIEWER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_RADAR_VIEWER))
#define DARXEN_RADAR_VIEWER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_RADAR_VIEWER, DarxenRadarViewerClass))

typedef struct _DarxenRadarViewer			DarxenRadarViewer;
typedef struct _DarxenRadarViewerClass		DarxenRadarViewerClass;
typedef struct _DarxenRadarViewerPrivate	DarxenRadarViewerPrivate;

struct _DarxenRadarViewer
{
	GtkDrawingArea darea;

	/*< private >*/
	DarxenRadarViewerPrivate *priv;
};

struct _DarxenRadarViewerClass
{
	GtkDrawingAreaClass parent_class;

	void (*frame_changed) (DarxenRadarViewer *radview);
};

GType      darxen_radar_viewer_get_type() G_GNUC_CONST;
GtkWidget* darxen_radar_viewer_new(SettingsView *viewSettings, SettingsSite *site);

/* Get */
gboolean				darxen_radar_viewer_get_smoothing(DarxenRadarViewer *radview);
gboolean				darxen_radar_viewer_get_frame_animate(DarxenRadarViewer *radview);
int						darxen_radar_viewer_get_frame_count(DarxenRadarViewer *radview);
int						darxen_radar_viewer_get_frame_current(DarxenRadarViewer *radview);
ProductsLevel3Data*		darxen_radar_viewer_get_current_frame_data(DarxenRadarViewer *radview);

/* Set */
void darxen_radar_viewer_set_smoothing(DarxenRadarViewer *radview, gboolean smooth);
void darxen_radar_viewer_set_frame_animate(DarxenRadarViewer *radview, gboolean animate);
void darxen_radar_viewer_set_frame_first(DarxenRadarViewer *radview);
void darxen_radar_viewer_set_frame_last(DarxenRadarViewer *radview);
void darxen_radar_viewer_set_frame_next(DarxenRadarViewer *radview);
void darxen_radar_viewer_set_frame_previous(DarxenRadarViewer *radview);

/* Utility Functions */
void darxen_radar_viewer_toggle_frame_animate(DarxenRadarViewer *radview);
void darxen_radar_viewer_append_frame(DarxenRadarViewer *radview, ProductsLevel3Data *objData);

G_END_DECLS

#endif /* __RADARVIEWER_H__ */

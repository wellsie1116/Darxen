/* darxenradarviewer.h
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

#ifndef DARXENRADARVIEWER_H_F8Q20G6S
#define DARXENRADARVIEWER_H_F8Q20G6S

#include <gltk/gltk.h>

#include "darxenconfig.h"

G_BEGIN_DECLS

#define DARXEN_RADAR_VIEWER_ERROR darxen_radar_viewer_error_quark()

#define DARXEN_TYPE_RADAR_VIEWER				(darxen_radar_viewer_get_type())
#define DARXEN_RADAR_VIEWER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_RADAR_VIEWER, DarxenRadarViewer))
#define DARXEN_RADAR_VIEWER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_RADAR_VIEWER, DarxenRadarViewerClass))
#define DARXEN_IS_RADAR_VIEWER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_RADAR_VIEWER))
#define DARXEN_IS_RADAR_VIEWER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_RADAR_VIEWER))
#define DARXEN_RADAR_VIEWER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_RADAR_VIEWER, DarxenRadarViewerClass))

typedef struct _DarxenRadarViewer			DarxenRadarViewer;
typedef struct _DarxenRadarViewerClass		DarxenRadarViewerClass;

struct _DarxenRadarViewer
{
	GltkWidget parent;
};

struct _DarxenRadarViewerClass
{
	GltkWidgetClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	DARXEN_RADAR_VIEWER_ERROR_FAILED
} DarxenRadarViewerError;

GType				darxen_radar_viewer_get_type	() G_GNUC_CONST;
DarxenRadarViewer*	darxen_radar_viewer_new			(const gchar* site, DarxenViewInfo* viewInfo);

/* Public functions here */

GQuark			darxen_radar_viewer_error_quark	();

G_END_DECLS

#endif


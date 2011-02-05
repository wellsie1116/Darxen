/*
 * libdarxenShapefileRenderer.h
 *
 *  Created on: Mar 12, 2010
 *      Author: wellska1
 */

#ifndef LIBDARXENSHAPEFILERENDERER_H_
#define LIBDARXENSHAPEFILERENDERER_H_

#include "libdarxenCommon.h"

#include "libdarxenShapefiles.h"
#include "libdarxenRadarSites.h"
#include "libdarxenRenderingCommon.h"

G_EXPORT void darxen_shapefile_renderer_render_text_labels	(DarxenShapefileTextLabels* labels, char* pathShp, DarxenRadarSiteInfo* siteInfo, DarxenGLFont* font, gboolean overlay);
G_EXPORT void darxen_shapefile_renderer_render_points		(DarxenShapefilePoints* points, char* pathShp, DarxenRadarSiteInfo* siteInfo, gboolean overlay);
G_EXPORT void darxen_shapefile_renderer_render_lines		(DarxenShapefileLines* lines, char* pathShp, DarxenRadarSiteInfo* siteInfo, gboolean overlay);
G_EXPORT void darxen_shapefile_renderer_render_polygons		(DarxenShapefilePolygons* polygons, char* pathShp, DarxenRadarSiteInfo* siteInfo, gboolean overlay);

#endif /* LIBDARXENSHAPEFILERENDERER_H_ */

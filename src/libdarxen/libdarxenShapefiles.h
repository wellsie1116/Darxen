/*
 * libdarxenShapefiles.h
 *
 *  Created on: Mar 10, 2010
 *      Author: wellska1
 */

#ifndef LIBDARXENSHAPEFILES_H_
#define LIBDARXENSHAPEFILES_H_

#include "libdarxenCommon.h"

#include <libxml/tree.h>
#include <glib/gslist.h>

typedef struct _DarxenShapefileTextLabels DarxenShapefileTextLabels;
typedef struct _DarxenShapefilePoints DarxenShapefilePoints;
typedef struct _DarxenShapefileLines DarxenShapefileLines;
typedef struct _DarxenShapefilePolygons DarxenShapefilePolygons;
typedef struct _DarxenShapefileDataLevel DarxenShapefileDataLevel;
typedef struct _DarxenShapefileDataLevels DarxenShapefileDataLevels;
typedef struct _DarxenShapefile DarxenShapefile;

struct _DarxenShapefileTextLabels
{
	gboolean visible;
	char* field;
	DarxenColor color;
	float fontSize;
};

struct _DarxenShapefilePoints
{
	gboolean visible;
	float radius;
	DarxenColor color;
};

struct _DarxenShapefileLines
{
	gboolean visible;
	float width;
	DarxenColor color;
};

struct _DarxenShapefilePolygons
{
	gboolean visible;
	DarxenColor color;
};

typedef enum
{
	DATALEVELS_COMPARISON_TYPE_MIN,
	DATALEVELS_COMPARISON_TYPE_MAX

} DarxenShapefileDataLevelsComparisonType;

struct _DarxenShapefileDataLevel
{
	float scale;
	float value;
};

struct _DarxenShapefileDataLevels
{
	char* field;
	DarxenShapefileDataLevelsComparisonType comparisonType;

	GSList* levels; /* DarxenShapefileDataLevel */
};

struct _DarxenShapefile
{
	char* name;
	gboolean visible;
	char* file;

	DarxenShapefileTextLabels* textLabels;
	DarxenShapefilePoints* points;
	DarxenShapefileLines* lines;
	DarxenShapefilePolygons* polygons;

	DarxenShapefileDataLevels* dataLevels;
};

G_EXPORT GSList* darxen_shapefiles_get_ids();
G_EXPORT DarxenShapefile* darxen_shapefiles_load_config(const xmlNode* node, DarxenShapefile* base);
G_EXPORT DarxenShapefile* darxen_shapefiles_load_by_id(const char* id);
G_EXPORT DarxenShapefile* darxen_shapefiles_clone(DarxenShapefile* src);
G_EXPORT void darxen_shapefiles_free_shapefile(DarxenShapefile* shapefile);

G_EXPORT char* darxen_shapefiles_filter_shp(DarxenShapefile* shapefile, int dataLevel, const char* site, float latCenter, float lonCenter, float radius);

#endif /* LIBDARXENSHAPEFILES_H_ */

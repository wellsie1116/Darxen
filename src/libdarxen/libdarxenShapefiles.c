/*
 * libdarxenShapefiles.c
 *
 *  Created on: Mar 10, 2010
 *      Author: wellska1
 */

#define G_LOG_DOMAIN "libdarxenShapefiles"
#include <glib/gmessages.h>

#include "libdarxenShapefiles.h"

#include <string.h>

#include <shapefil.h>

#include <libxml/xpath.h>
#include <glib.h>

#include "libdarxenXml.h"
#include "libdarxenFileSupport.h"

static GList* shapefiles = NULL;
static GSList* shapefileIds = NULL;

static void		load_shapefiles			();
static gboolean	create_field			(char* name, DBFHandle base, DBFHandle new);
static gboolean	create_required_fields	(DarxenShapefile* shapefile, DBFHandle base, DBFHandle new);
static gboolean	write_field_data		(DBFHandle base, DBFHandle new, int baseRecordIndex, int newRecordIndex, int baseFieldIndex, int newFieldIndex);
static gboolean	write_fields			(DarxenShapefile* shapefile, int baseRecordIndex, int newRecordIndex, DBFHandle base, DBFHandle new);

/********************
 * Public Functions *
 ********************/

GSList*
darxen_shapefiles_get_ids()
{
	load_shapefiles();

	return shapefileIds;
}

/**
 * Reads the shapefile configuration from the given xmlNode, using the supplied base shapefile config
 * as the default values (or NULL)
 *
 * if base is supplied, it will be modified in place and returned! Otherwise, a new shapefile config
 * object will be created and returned.
 */
DarxenShapefile*
darxen_shapefiles_load_config(const xmlNode* node, DarxenShapefile* base)
{
	load_shapefiles();
	xmlDoc* doc;
	xmlNode* newNode;
	xmlXPathContext* xpath;

	doc = xmlNewDoc((xmlChar*)"1.0");
	g_assert(doc);
	newNode = xmlDocCopyNode((xmlNode*)node, doc, 1);
	g_assert(newNode);
	xmlDocSetRootElement(doc, newNode);

	xpath = xmlXPathNewContext(doc);
	if (!xpath)
	{
		xmlFreeDoc(doc);
		return NULL;
	}

	int i;
	DarxenShapefile* shapefile;
	if (base)
	{
		shapefile = base;

		shapefile->name = darxen_xml_try_get_string(xpath, base->name, "/Shapefile/@id");
		shapefile->file = darxen_xml_try_get_string(xpath, base->file, "/Shapefile/File/text()");
		shapefile->visible = darxen_xml_try_get_boolean(xpath, base->visible, "/Shapefile/@visible");

		gboolean hasTextLabels = (int)darxen_xml_get_number(xpath, "count(/Shapefile/TextLabels/node())") > 0;
		gboolean hasPoints = (int)darxen_xml_get_number(xpath, "count(/Shapefile/Points/node())") > 0;
		gboolean hasLines = (int)darxen_xml_get_number(xpath, "count(/Shapefile/Lines/node())") > 0;
		gboolean hasPolygons = (int)darxen_xml_get_number(xpath, "count(/Shapefile/Polygons/node())") > 0;

		gboolean hasDataLevels = (int)darxen_xml_get_number(xpath, "count(/Shapefile/DataLevels/node())") > 0;

		if (base->textLabels)
			shapefile->textLabels->visible = darxen_xml_try_get_boolean(xpath, base->textLabels->visible, "/Shapefile/TextLabels/@visible");
		if (base->points)
			shapefile->points->visible = darxen_xml_try_get_boolean(xpath, base->points->visible, "/Shapefile/Points/@visible");
		if (base->lines)
			shapefile->lines->visible = darxen_xml_try_get_boolean(xpath, base->lines->visible, "/Shapefile/Lines/@visible");
		if (base->polygons)
			shapefile->polygons->visible = darxen_xml_try_get_boolean(xpath, base->polygons->visible, "/Shapefile/Polygons/@visible");

		if (base->dataLevels)
		{
			shapefile->dataLevels->field = darxen_xml_try_get_string(xpath, base->dataLevels->field, "/Shapefile/DataLevels/@field");

			char* comparisonType = darxen_xml_try_get_string(xpath, NULL, "/Shapefile/DataLevels/@comparison-type");
			if (comparisonType)
			{
				if (!strcmp(comparisonType, "min"))
					shapefile->dataLevels->comparisonType = DATALEVELS_COMPARISON_TYPE_MIN;
				else if (!strcmp(comparisonType, "max"))
					shapefile->dataLevels->comparisonType = DATALEVELS_COMPARISON_TYPE_MAX;
				else
					g_warning("Invalid comparison type (%s), using default", comparisonType);
				free(comparisonType);
			}
		}

		if (hasTextLabels)
		{
			shapefile->textLabels->field = darxen_xml_try_get_string(xpath, base->textLabels->field, "/Shapefile/TextLabels/Field/text()");
			if (!darxen_xml_get_color(xpath, shapefile->textLabels->color, "/Shapefile/TextLabels/Font/Color"))
				for (i = 0; i < 4; i++)
					shapefile->textLabels->color[i] = base->textLabels->color[i];
			shapefile->textLabels->fontSize = darxen_xml_try_get_number(xpath, base->textLabels->fontSize, "/Shapefile/TextLabels/Font/Size/text()");
		}
		if (hasPoints)
		{
			shapefile->points->radius = darxen_xml_try_get_number(xpath, base->points->radius, "/Shapefile/TextLabels/Radius/text()");
			if (!darxen_xml_get_color(xpath, shapefile->points->color, "/Shapefile/TextLabels/Font/Color"))
				for (i = 0; i < 4; i++)
					shapefile->points->color[i] = base->points->color[i];
		}
		if (hasLines)
		{
			shapefile->lines->width = darxen_xml_try_get_number(xpath, base->lines->width, "/Shapefile/TextLabels/Width/text()");
			if (!darxen_xml_get_color(xpath, shapefile->lines->color, "/Shapefile/TextLabels/Font/Color"))
				for (i = 0; i < 4; i++)
					shapefile->lines->color[i] = base->lines->color[i];
		}
		if (hasPolygons)
		{
			if (!darxen_xml_get_color(xpath, shapefile->polygons->color, "/Shapefile/TextLabels/Font/Color"))
				for (i = 0; i < 4; i++)
					shapefile->polygons->color[i] = base->polygons->color[i];
		}
		if (hasDataLevels)
		{
			int levelIndex;
			int levelCount = (int)darxen_xml_get_number(xpath, "count(/Shapefile/DataLevels/DataLevel)");
			if (levelCount > 0)
			{
				GSList* pLevels = shapefile->dataLevels->levels;
				while (pLevels)
				{
					free(pLevels->data);
					pLevels = pLevels->next;
				}
				g_slist_free(shapefile->dataLevels->levels);
				shapefile->dataLevels->levels = NULL;

				for (levelIndex = 1; levelIndex <= levelCount; levelIndex++)
				{
					DarxenShapefileDataLevel* level = (DarxenShapefileDataLevel*)malloc(sizeof(DarxenShapefileDataLevel));
					shapefile->dataLevels->levels = g_slist_append(shapefile->dataLevels->levels, level);

					gchar* expr = darxen_xml_path_format("/Shapefile/DataLevels/DataLevel[%i]/Scale", levelIndex);
					level->scale = (float)darxen_xml_get_number(xpath, expr); g_free(expr);
					expr = darxen_xml_path_format("/Shapefile/DataLevels/DataLevel[%i]/Value", levelIndex);
					level->value = (float)darxen_xml_get_number(xpath, expr); g_free(expr);
				}
			}
		}

	}
	else
	{
		shapefile = (DarxenShapefile*)malloc(sizeof(DarxenShapefile));

		shapefile->name = darxen_xml_get_string(xpath, "/Shapefile/@id");
		shapefile->file = darxen_xml_get_string(xpath, "/Shapefile/File/text()");
		shapefile->visible = darxen_xml_try_get_boolean(xpath, FALSE, "/Shapefile/@visible");

		gboolean hasTextLabels = (int)darxen_xml_get_number(xpath, "count(/Shapefile/TextLabels/node())") > 0;
		gboolean hasPoints = (int)darxen_xml_get_number(xpath, "count(/Shapefile/Points/node())") > 0;
		gboolean hasLines = (int)darxen_xml_get_number(xpath, "count(/Shapefile/Lines/node())") > 0;
		gboolean hasPolygons = (int)darxen_xml_get_number(xpath, "count(/Shapefile/Polygons/node())") > 0;

		gboolean hasDataLevels = (int)darxen_xml_get_number(xpath, "count(/Shapefile/DataLevels/node())") > 0;

		shapefile->textLabels = NULL;
		shapefile->points = NULL;
		shapefile->lines = NULL;
		shapefile->polygons = NULL;

		shapefile->dataLevels = NULL;

		if (hasTextLabels)
		{
			shapefile->textLabels = (DarxenShapefileTextLabels*)malloc(sizeof(DarxenShapefileTextLabels));
			shapefile->textLabels->visible = darxen_xml_try_get_boolean(xpath, FALSE, "/Shapefile/TextLabels/@visible");
			shapefile->textLabels->field = darxen_xml_get_string(xpath, "/Shapefile/TextLabels/Field/text()");
			darxen_xml_get_color(xpath, shapefile->textLabels->color, "/Shapefile/TextLabels/Font/Color");
			shapefile->textLabels->fontSize = darxen_xml_get_number(xpath, "/Shapefile/TextLabels/Font/Size/text()");
		}
		if (hasPoints)
		{
			shapefile->points = (DarxenShapefilePoints*)malloc(sizeof(DarxenShapefilePoints));
			shapefile->points->visible = darxen_xml_try_get_boolean(xpath, FALSE, "/Shapefile/Points/@visible");
			shapefile->points->radius = darxen_xml_get_number(xpath, "/Shapefile/Points/Radius/text()");
			darxen_xml_get_color(xpath, shapefile->points->color, "/Shapefile/Points/Color");
		}
		if (hasLines)
		{
			shapefile->lines = (DarxenShapefileLines*)malloc(sizeof(DarxenShapefileLines));
			shapefile->lines->visible = darxen_xml_try_get_boolean(xpath, FALSE, "/Shapefile/Lines/@visible");
			shapefile->lines->width = darxen_xml_get_number(xpath, "/Shapefile/Lines/Width/text()");
			darxen_xml_get_color(xpath, shapefile->lines->color, "/Shapefile/Lines/Color");
		}
		if (hasPolygons)
		{
			shapefile->polygons = (DarxenShapefilePolygons*)malloc(sizeof(DarxenShapefilePolygons));
			shapefile->polygons->visible = darxen_xml_try_get_boolean(xpath, FALSE, "/Shapefile/Polygons/@visible");
			darxen_xml_get_color(xpath, shapefile->polygons->color, "/Shapefile/Polygons/Color");
		}
		if (hasDataLevels)
		{
			shapefile->dataLevels = (DarxenShapefileDataLevels*)malloc(sizeof(DarxenShapefileDataLevels));
			shapefile->dataLevels->field = darxen_xml_get_string(xpath, "/Shapefile/DataLevels/@field");
			char* comparisonType = darxen_xml_get_string(xpath, "/Shapefile/DataLevels/@comparison-type");
			shapefile->dataLevels->comparisonType = DATALEVELS_COMPARISON_TYPE_MIN;
			if (!strcmp(comparisonType, "min"))
				shapefile->dataLevels->comparisonType = DATALEVELS_COMPARISON_TYPE_MIN;
			else if (!strcmp(comparisonType, "max"))
				shapefile->dataLevels->comparisonType = DATALEVELS_COMPARISON_TYPE_MAX;
			else
				g_warning("Invalid comparison type (%s), defaulting to min", comparisonType);
			free(comparisonType);

			shapefile->dataLevels->levels = NULL;
			int levelIndex;
			int levelCount = (int)darxen_xml_get_number(xpath, "count(/Shapefile/DataLevels/DataLevel)");
			for (levelIndex = 1; levelIndex <= levelCount; levelIndex++)
			{
				DarxenShapefileDataLevel* level = (DarxenShapefileDataLevel*)malloc(sizeof(DarxenShapefileDataLevel));
				shapefile->dataLevels->levels = g_slist_append(shapefile->dataLevels->levels, level);

				gchar* expr = darxen_xml_path_format("/Shapefile/DataLevels/DataLevel[%i]/Scale", levelIndex);
				level->scale = (float)darxen_xml_get_number(xpath, expr); g_free(expr);
				expr = darxen_xml_path_format("/Shapefile/DataLevels/DataLevel[%i]/Value", levelIndex);
				level->value = (float)darxen_xml_get_number(xpath, expr); g_free(expr);
			}
		}
	}

	xmlXPathFreeContext(xpath);
	xmlFreeDoc(doc);

	return shapefile;
}

DarxenShapefile*
darxen_shapefiles_load_by_id(const char* id)
{
	load_shapefiles();

	DarxenShapefile* found = NULL;

	GList* pShapefiles = shapefiles;
	while (pShapefiles && !found)
	{
		DarxenShapefile* shapefile = (DarxenShapefile*)pShapefiles->data;

		if (!strcmp(shapefile->name, id))
			found = shapefile;

		pShapefiles = pShapefiles->next;
	}

	if (!found)
		return NULL; //id does not exist

	return darxen_shapefiles_clone(found);
}

DarxenShapefile*
darxen_shapefiles_clone(DarxenShapefile* src)
{
	int i;
	DarxenShapefile* res = (DarxenShapefile*)malloc(sizeof(DarxenShapefile));

	res->name = strdup(src->name);
	res->file = strdup(src->file);
	res->visible = src->visible;

	if (src->textLabels)
	{
		res->textLabels = (DarxenShapefileTextLabels*)malloc(sizeof(DarxenShapefileTextLabels));
		res->textLabels->visible = src->textLabels->visible;
		res->textLabels->field = strdup(src->textLabels->field);
		for (i = 0; i < 4; i++)
			res->textLabels->color[i] = src->textLabels->color[i];
		res->textLabels->fontSize = src->textLabels->fontSize;
	}
	else
	{
		res->textLabels = NULL;
	}

	if (src->points)
	{
		res->points = (DarxenShapefilePoints*)malloc(sizeof(DarxenShapefilePoints));
		res->points->visible = src->points->visible;
		res->points->radius = src->points->radius;
		for (i = 0; i < 4; i++)
			res->points->color[i] = src->points->color[i];
	}
	else
	{
		res->points = NULL;
	}

	if (src->lines)
	{
		res->lines = (DarxenShapefileLines*)malloc(sizeof(DarxenShapefileLines));
		res->lines->visible = src->lines->visible;
		res->lines->width = src->lines->width;
		for (i = 0; i < 4; i++)
			res->lines->color[i] = src->lines->color[i];
	}
	else
	{
		res->lines = NULL;
	}

	if (src->polygons)
	{
		res->polygons = (DarxenShapefilePolygons*)malloc(sizeof(DarxenShapefilePolygons));
		res->polygons->visible = src->polygons->visible;
		for (i = 0; i < 4; i++)
			res->polygons->color[i] = src->polygons->color[i];
	}
	else
	{
		res->polygons = NULL;
	}

	if (src->dataLevels)
	{
		res->dataLevels = (DarxenShapefileDataLevels*)malloc(sizeof(DarxenShapefileDataLevels));
		res->dataLevels->field = strdup(src->dataLevels->field);
		res->dataLevels->comparisonType = src->dataLevels->comparisonType;

		res->dataLevels->levels = NULL;
		GSList* pLevels = src->dataLevels->levels;
		while (pLevels)
		{
			DarxenShapefileDataLevel* foundLevel = (DarxenShapefileDataLevel*)pLevels->data;
			DarxenShapefileDataLevel* newLevel = (DarxenShapefileDataLevel*)malloc(sizeof(DarxenShapefileDataLevel));

			newLevel->scale = foundLevel->scale;
			newLevel->value = foundLevel->value;

			res->dataLevels->levels = g_slist_append(res->dataLevels->levels, newLevel);

			pLevels = pLevels->next;
		}
	}
	else
	{
		res->dataLevels = NULL;
	}

	return res;
}

void
darxen_shapefiles_free_shapefile(DarxenShapefile* shapefile)
{
	if (shapefile->textLabels)
		free(shapefile->textLabels);
	if (shapefile->points)
		free(shapefile->points);
	if (shapefile->lines)
		free(shapefile->lines);
	if (shapefile->polygons)
		free(shapefile->polygons);
	if (shapefile->dataLevels)
	{
		GSList* pLevels = shapefile->dataLevels->levels;
		while (pLevels)
		{
			free(pLevels->data);
			pLevels = pLevels->next;
		}
		g_slist_free(shapefile->dataLevels->levels);
		free(shapefile->dataLevels);
	}
	free(shapefile);
}

char*
darxen_shapefiles_filter_shp(DarxenShapefile* shapefile, int dataLevel, const char* site, float latCenter, float lonCenter, float radius)
{
	GSList* lstNewDBFs = NULL;
	GSList* lstNewSHPs = NULL;
	DBFHandle hDBF;
	SHPHandle hSHP;
	SHPHandle hNewSHP = NULL;
	DBFHandle hNewDBF = NULL;
	int pnEntities;
	int pnShapeType;
	int i;
	SHPObject *psObject = NULL;
	char* filteredPath;

	g_assert(shapefile->file);
	g_assert(!shapefile->dataLevels || shapefile->dataLevels->field);
	g_assert(!shapefile->dataLevels || (dataLevel >= 0 && dataLevel < g_slist_length(shapefile->dataLevels->levels)));

	gchar* gfilteredFile;
	if (shapefile->dataLevels)
		gfilteredFile = g_strdup_printf("%s_%s_%i", shapefile->file, site, dataLevel);
	else
		gfilteredFile = g_strdup_printf("%s_%s", shapefile->file, site);
	gchar* gfilteredPath = g_build_filename(darxen_file_support_get_app_path(), "shapefiles", "cache", gfilteredFile, NULL);
	filteredPath = strdup(gfilteredPath);
	g_free(gfilteredFile);
	g_free(gfilteredPath);

	//don't recreate the file, just return the path
	gchar* shpFile = g_strdup_printf("%s.shp", filteredPath);
	gchar* dbfFile = g_strdup_printf("%s.dbf", filteredPath);
	if (g_file_test(shpFile, G_FILE_TEST_EXISTS) && g_file_test(dbfFile, G_FILE_TEST_EXISTS))
	{
		g_free(shpFile);
		g_free(dbfFile);
		return filteredPath;
	}
	g_free(shpFile);
	g_free(dbfFile);

	gchar* pathPart = g_strdup_printf("%s.shp", shapefile->file);
	gchar* pathPart2 = g_build_filename("shapefiles", pathPart, NULL);
	gchar* shapefilePath = darxen_file_support_get_overridable_file_path(pathPart2);
		//g_build_filename(darxen_file_support_get_app_path(), "shapefiles", shapefile->file, NULL);
	g_free(pathPart);
	g_free(pathPart2);
	g_assert(shapefilePath);
	shapefilePath[strlen(shapefilePath)-4] = '\0';
	hSHP = SHPOpen(shapefilePath, "rb");
	if (!hSHP)
	{
		g_free(shapefilePath);
		g_critical("Invalid shapefile path: %s", shapefile->file);
		return NULL;
	}
	hDBF = DBFOpen(shapefilePath, "rb");
	if (!hDBF)
	{
		g_free(shapefilePath);
		g_critical("Invalid shapefile dbf path: %s", shapefile->file);
		return NULL;
	}
	g_free(shapefilePath);

	int dbfCount = DBFGetRecordCount(hDBF);
	SHPGetInfo(hSHP, &pnEntities, &pnShapeType, NULL, NULL);
	if (dbfCount != pnEntities)
	{
		g_critical("dbf and shp have a differing number of records!");
		SHPClose(hSHP);
		DBFClose(hDBF);
		return NULL;
	}

	if (shapefile->dataLevels)
	{
		GSList* pLevel = shapefile->dataLevels->levels;
		i = 0;
		while (pLevel)
		{
			gfilteredFile = g_strdup_printf("%s_%s_%i", shapefile->file, site, i);
			gfilteredPath = g_build_filename(darxen_file_support_get_app_path(), "shapefiles", "cache", gfilteredFile, NULL);
			hNewDBF = DBFCreate(gfilteredPath);
			hNewSHP = SHPCreate(gfilteredPath, pnShapeType);
			if (!hNewDBF || !hNewSHP || !create_required_fields(shapefile, hDBF, hNewDBF))
			{
				SHPClose(hSHP);
				DBFClose(hDBF);
				if (hNewDBF)
					DBFClose(hNewDBF);
				if (hNewSHP)
					SHPClose(hNewSHP);
				GSList* plstNewDBFs = lstNewDBFs;
				while (plstNewDBFs)
				{
					DBFClose((DBFHandle)plstNewDBFs->data);
					plstNewDBFs = plstNewDBFs->next;
				}
				g_slist_free(lstNewDBFs);
				GSList* plstNewSHPs = lstNewSHPs;
				while (plstNewSHPs)
				{
					SHPClose((SHPHandle)plstNewSHPs->data);
					plstNewSHPs = plstNewSHPs->next;
				}
				g_slist_free(lstNewSHPs);
				g_critical("Unable to create filtered shapefile lists: (level %i)", i);
				return NULL;
			}
			lstNewDBFs = g_slist_append(lstNewDBFs, hNewDBF);
			lstNewSHPs = g_slist_append(lstNewSHPs, hNewSHP);
			g_free(gfilteredPath);
			g_free(gfilteredFile);
			i++;
			pLevel = pLevel->next;
		}
		hNewDBF = NULL;
		hNewSHP = NULL;
	}
	else
	{
		hNewSHP = SHPCreate(filteredPath, pnShapeType);
		if (!hNewSHP)
		{
			SHPClose(hSHP);
			DBFClose(hDBF);
			g_critical("Unable to create filtered shapefile: %s", filteredPath);
			return NULL;
		}

		hNewDBF = DBFCreate(filteredPath);
		if (!hNewDBF || !create_required_fields(shapefile, hDBF, hNewDBF))
		{
			SHPClose(hSHP);
			DBFClose(hDBF);
			SHPClose(hNewSHP);
			g_critical("Unable to create filtered dbf shapefile: %s", filteredPath);
			return NULL;
		}
	}

	float filterRegionX1 = lonCenter - radius;
	float filterRegionX2 = lonCenter + radius;
	float filterRegionY1 = latCenter - radius;
	float filterRegionY2 = latCenter + radius;

	for (i = 0; i < pnEntities; i++)
	{
		psObject = SHPReadObject(hSHP, i);

		if (((filterRegionX1 >= psObject->dfXMin && filterRegionX1 <= psObject->dfXMax) ||
			 (filterRegionX2 >= psObject->dfXMin && filterRegionX2 <= psObject->dfXMax) ||
			 (psObject->dfXMin >= filterRegionX1 && psObject->dfXMin <= filterRegionX2) ||
			 (psObject->dfXMax >= filterRegionX1 && psObject->dfXMax <= filterRegionX2)) &&
			((filterRegionY1 >= psObject->dfYMin && filterRegionY1 <= psObject->dfYMax) ||
			 (filterRegionY2 >= psObject->dfYMin && filterRegionY2 <= psObject->dfYMax) ||
			 (psObject->dfYMin >= filterRegionY1 && psObject->dfYMin <= filterRegionY2) ||
			 (psObject->dfYMax >= filterRegionY1 && psObject->dfYMax <= filterRegionY2)))
		{
			psObject->nShapeId = -1;

			if (shapefile->dataLevels)
			{
				int fieldIndex = DBFGetFieldIndex(hDBF, shapefile->dataLevels->field);
				GSList* pDataLevels = shapefile->dataLevels->levels;
				GSList* pLstNewSHPs = lstNewSHPs;
				GSList* pLstNewDBFs = lstNewDBFs;
				while (pDataLevels)
				{
					DarxenShapefileDataLevel* level = (DarxenShapefileDataLevel*)pDataLevels->data;

					hNewSHP = (SHPHandle)pLstNewSHPs->data;
					hNewDBF = (DBFHandle)pLstNewDBFs->data;

					double value = DBFReadDoubleAttribute(hDBF, i, fieldIndex);

					gboolean cond;
					switch (shapefile->dataLevels->comparisonType)
					{
					case DATALEVELS_COMPARISON_TYPE_MIN:
						cond = (level->value < value);
						break;
					case DATALEVELS_COMPARISON_TYPE_MAX:
						cond = (level->value > value);
						break;
					default:
						g_warning("Invalid comparison type: %i", shapefile->dataLevels->comparisonType);
						cond = TRUE;
					}

					if (cond)
					{
						int index = SHPWriteObject(hNewSHP, -1, psObject);
						write_fields(shapefile, i, index, hDBF, hNewDBF);
						break;
					}

					pDataLevels = pDataLevels->next;
					pLstNewDBFs = pLstNewDBFs->next;
					pLstNewSHPs = pLstNewSHPs->next;
				}
				hNewDBF = NULL;
				hNewSHP = NULL;
			}
			else
			{
				int index = SHPWriteObject(hNewSHP, -1, psObject);
				write_fields(shapefile, i, index, hDBF, hNewDBF);
			}
		}
		SHPDestroyObject(psObject);
	}


	SHPClose(hSHP);
	DBFClose(hDBF);
	if (hNewDBF)
		DBFClose(hNewDBF);
	if (hNewSHP)
		SHPClose(hNewSHP);
	GSList* plstNewDBFs = lstNewDBFs;
	while (plstNewDBFs)
	{
		DBFClose((DBFHandle)plstNewDBFs->data);
		plstNewDBFs = plstNewDBFs->next;
	}
	g_slist_free(lstNewDBFs);
	GSList* plstNewSHPs = lstNewSHPs;
	while (plstNewSHPs)
	{
		SHPClose((SHPHandle)plstNewSHPs->data);
		plstNewSHPs = plstNewSHPs->next;
	}
	g_slist_free(lstNewSHPs);

//	g_assert(g_file_test(filteredPath, G_FILE_TEST_EXISTS));

	return filteredPath;
}

/*********************
 * Private Functions *
 *********************/

static void
load_shapefiles()
{
	if (shapefiles || shapefileIds)
		return;

	xmlDoc *doc;
	xmlXPathContext* xpath;

	doc = xmlReadFile(darxen_file_support_get_overridable_file_path("Shapefiles.xml"), NULL, 0);

	if (!doc)
	{
		g_warning("Unable to load default shapefiles");
		return;
	}

	xpath = xmlXPathNewContext(doc);
	if (!xpath)
	{
		xmlFreeDoc(doc);
		g_warning("Unable to load default shapefiles");
		return;
	}

	int idCount;
	char** ids = darxen_xml_get_strs(xpath, "/Shapefiles/Shapefile/@id", &idCount);

	int i;
	for (i = 0; i < idCount; i++)
	{
		shapefileIds = g_slist_prepend(shapefileIds, ids[1]);

		gchar* expr = darxen_xml_path_format("/Shapefiles/Shapefile[@id=\"%s\"]", ids[i]);
		xmlNodeSet* nodes = darxen_xml_get_nodes(xpath, expr);
		g_free(expr);
		g_assert(nodes->nodeNr == 1);

		DarxenShapefile* shapefile = darxen_shapefiles_load_config(nodes->nodeTab[0], NULL);

		shapefiles = g_list_prepend(shapefiles, shapefile);

		xmlXPathFreeNodeSet(nodes);
	}
	free(ids);

	shapefileIds = g_slist_reverse(shapefileIds);
	shapefiles = g_list_reverse(shapefiles);

    xmlXPathFreeContext(xpath);
    xmlFreeDoc(doc);
}

static gboolean
create_field(char* name, DBFHandle base, DBFHandle new)
{
	int field;
	char fieldName[12];
	int fieldWidth;
	int fieldDecimals;
	DBFFieldType fieldType;

	field = DBFGetFieldIndex(base, name);
	if (field < 0)
	{
		g_critical("Invalid shapefile field name: %s", name);
		return FALSE;
	}
	fieldType = DBFGetFieldInfo(base, field, fieldName, &fieldWidth, &fieldDecimals);
	DBFAddField(new, fieldName, fieldType, fieldWidth, fieldDecimals);

	return TRUE;
}

static gboolean
create_required_fields(DarxenShapefile* shapefile, DBFHandle base, DBFHandle new)
{
	if (shapefile->textLabels)
		if (!create_field(shapefile->textLabels->field, base, new))
			return FALSE;
	if (shapefile->dataLevels)
		if (!create_field(shapefile->dataLevels->field, base, new))
			return FALSE;
	return TRUE;
}

static gboolean
write_field_data(DBFHandle base, DBFHandle new, int baseRecordIndex, int newRecordIndex, int baseFieldIndex, int newFieldIndex)
{
	switch (DBFGetFieldInfo(base, baseFieldIndex, NULL, NULL, NULL))
	{
	case FTString:
		DBFWriteStringAttribute(new, newRecordIndex, newFieldIndex, DBFReadStringAttribute(base, baseRecordIndex, baseFieldIndex));
		break;
	case FTInteger:
		DBFWriteIntegerAttribute(new, newRecordIndex, newFieldIndex, DBFReadIntegerAttribute(base, baseRecordIndex, baseFieldIndex));
		break;
	case FTDouble:
		DBFWriteDoubleAttribute(new, newRecordIndex, newFieldIndex, DBFReadDoubleAttribute(base, baseRecordIndex, baseFieldIndex));
		break;
//	case FTLogical:
//		DBFWriteLogicalAttribute(new, newRecordIndex, newFieldIndex, DBFReadLogicalAttribute(base, baseRecordIndex, baseFieldIndex));
//		break;
	default:
		g_critical("Invalid field type in write_field");
		return FALSE;
	}
	return TRUE;
}

static gboolean
write_fields(DarxenShapefile* shapefile, int baseRecordIndex, int newRecordIndex, DBFHandle base, DBFHandle new)
{
	int baseFieldIndex;
	int newFieldIndex;

	if (shapefile->textLabels)
	{
		baseFieldIndex = DBFGetFieldIndex(base, shapefile->textLabels->field);
		newFieldIndex = DBFGetFieldIndex(new, shapefile->textLabels->field);

		if (!write_field_data(base, new, baseRecordIndex, newRecordIndex, baseFieldIndex, newFieldIndex))
			return FALSE;
	}
	if (shapefile->dataLevels)
	{
		baseFieldIndex = DBFGetFieldIndex(base, shapefile->dataLevels->field);
		newFieldIndex = DBFGetFieldIndex(new, shapefile->dataLevels->field);

		if (!write_field_data(base, new, baseRecordIndex, newRecordIndex, baseFieldIndex, newFieldIndex))
			return FALSE;
	}

	return TRUE;
}




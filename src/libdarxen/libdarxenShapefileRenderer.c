/*
 * libdarxenShapefileRenderer.c
 *
 *  Created on: Mar 12, 2010
 *      Author: wellska1
 */

#define G_LOG_DOMAIN "libdarxenShapefileRenderer"
#include <glib/gmessages.h>

#include "libdarxenShapefileRenderer.h"

#include "libdarxenRenderingCommon.h"
#include "libdarxenRadarSites.h"

#include <glib/gtestutils.h>
#include <shapefil.h>

static GSList* lstMemCombine = NULL;

static void APIENTRY tessBegin(GLenum type);
static void APIENTRY tessVertex(GLvoid *vertex);
static void APIENTRY tessEnd();
static void APIENTRY tessError(GLenum errno);
static void APIENTRY tessCombine(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], GLdouble **dataOut);

void
darxen_shapefile_renderer_render_text_labels(DarxenShapefileTextLabels* labels, char* pathShp, DarxenRadarSiteInfo* siteInfo, DarxenGLFont* font, gboolean overlay)
{
	float latCenter = siteInfo->fltLatitude;
	float lonCenter = siteInfo->fltLongitude;

	SHPHandle hSHP;
	DBFHandle hDBF;
	SHPObject *psObject;
	int pnEntities;
	int pnShapeType;
	int records;
	int fieldIndex;
	int i;

	hSHP = SHPOpen(pathShp, "rb");
	if (!hSHP)
		return;
	hDBF = DBFOpen(pathShp, "rb");
	if (!hDBF)
	{
		SHPClose(hSHP);
		return;
	}

	SHPGetInfo(hSHP, &pnEntities, &pnShapeType, NULL, NULL);
	records = DBFGetRecordCount(hDBF);
	g_assert(pnEntities == records);

	fieldIndex = DBFGetFieldIndex(hDBF, labels->field);

	for (i = 0; i < records; i++)
	{
		psObject = SHPReadObject(hSHP, i);

		g_assert(psObject->nVertices == 1);

		float* pt = darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, *(psObject->padfY), *(psObject->padfX));

		darxen_rendering_common_draw_string(DBFReadStringAttribute(hDBF, i, fieldIndex), TEXT_ORIGIN_UPPER_LEFT,
											pt[0], pt[1], font, NULL, FALSE);

		SHPDestroyObject(psObject);
	}

	SHPClose(hSHP);
	DBFClose(hDBF);
}

void
darxen_shapefile_renderer_render_points(DarxenShapefilePoints* points, char* pathShp, DarxenRadarSiteInfo* siteInfo, gboolean overlay)
{
	float latCenter = siteInfo->fltLatitude;
	float lonCenter = siteInfo->fltLongitude;

	glPointSize(points->radius);

	SHPHandle hSHP;
	int pnEntities;
	int pnShapeType;
	int i;
	int j;
	SHPObject *psObject;
	glEnable(GL_POINT_SMOOTH);
	glBegin(GL_POINTS);
	{
		hSHP = SHPOpen(pathShp, "rb");
		SHPGetInfo(hSHP, &pnEntities, &pnShapeType, NULL, NULL);

		for (i = 0; i < pnEntities; i++)
		{
			psObject = SHPReadObject(hSHP, i);

			for (j = 0; j < psObject->nVertices; j++)
			{
				glVertex2fv(darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, psObject->padfY[j], psObject->padfX[j]));
			}

			SHPDestroyObject(psObject);
		}
		SHPClose(hSHP);
	}
	glEnd();
	glDisable(GL_POINT_SMOOTH);
}

void
darxen_shapefile_renderer_render_lines(DarxenShapefileLines* lines, char* pathShp, DarxenRadarSiteInfo* siteInfo, gboolean overlay)
{
	float latCenter = siteInfo->fltLatitude;
	float lonCenter = siteInfo->fltLongitude;

	glLineWidth(lines->width);

	SHPHandle hSHP;
	int pnEntities;
	int pnShapeType;
	int i;
	int j;
	int k;
	SHPObject *psObject;

	hSHP = SHPOpen(pathShp, "rb");
	SHPGetInfo(hSHP, &pnEntities, &pnShapeType, NULL, NULL);

	for (i = 0; i < pnEntities; i++)
	{
		psObject = SHPReadObject(hSHP, i);

		for (j = 0; j < psObject->nParts; j++)
		{
			glBegin(GL_LINE_STRIP);
			{
				for (k = psObject->panPartStart[j]; k < psObject->nVertices; k++)
				{
					glVertex2fv(darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, psObject->padfY[k], psObject->padfX[k]));
					if (j < psObject->nParts - 1 && k + 1 == psObject->panPartStart[j + 1])
						break;
				}
			}
			glEnd();
		}

		SHPDestroyObject(psObject);
	}
	SHPClose(hSHP);
}

void
darxen_shapefile_renderer_render_polygons(DarxenShapefilePolygons* polygons, char* pathShp, DarxenRadarSiteInfo* siteInfo, gboolean overlay)
{
	float latCenter = siteInfo->fltLatitude;
	float lonCenter = siteInfo->fltLongitude;


	SHPHandle hSHP;
	int pnEntities;
	int pnShapeType;
	int i;
	int j;
	int k;
	SHPObject* psObject;
	GLUtesselator* tess;

	hSHP = SHPOpen(pathShp, "rb");
	SHPGetInfo(hSHP, &pnEntities, &pnShapeType, NULL, NULL);

	tess = gluNewTess();

	gluTessCallback(tess, GLU_TESS_BEGIN, tessBegin);
	gluTessCallback(tess, GLU_TESS_VERTEX, tessVertex);
	gluTessCallback(tess, GLU_TESS_END, tessEnd);
	gluTessCallback(tess, GLU_TESS_ERROR, tessError);
	gluTessCallback(tess, GLU_TESS_COMBINE, tessCombine);

	for (i = 0; i < pnEntities; i++)
	{
		psObject = SHPReadObject(hSHP, i);

		double* polygonPoints = (double*)calloc(psObject->nVertices * 3, sizeof(double));
		double* pPolygonPoints = polygonPoints;

		gluTessBeginPolygon(tess, NULL);
		for (j = 0; j < psObject->nParts; j++)
		{
			gluTessBeginContour(tess);
			for (k = psObject->panPartStart[j]; k < psObject->nVertices; k++)
			{
				float* convPoint = darxen_rendering_common_convert_lon_lat_to_x_y(latCenter, lonCenter, psObject->padfY[k], psObject->padfX[k]);
				pPolygonPoints[0] = convPoint[0];
				pPolygonPoints[1] = convPoint[1];
				pPolygonPoints[2] = 0.0;
				gluTessVertex(tess, pPolygonPoints, pPolygonPoints);
				pPolygonPoints += 3;
				if (j < psObject->nParts - 1 && k + 1 == psObject->panPartStart[j + 1])
					break;
			}
			gluTessEndContour(tess);
		}
		gluTessEndPolygon(tess);

		free(polygonPoints);
		GSList* pLstMemCombine = lstMemCombine;
		while (pLstMemCombine)
		{
			free(pLstMemCombine->data);
			pLstMemCombine = pLstMemCombine->next;
		}
		g_slist_free(lstMemCombine);
		lstMemCombine = NULL;

		SHPDestroyObject(psObject);
	}

	gluDeleteTess(tess);
	SHPClose(hSHP);
}

/*********************
 * Private Functions *
 *********************/

static void APIENTRY
tessBegin(GLenum type)
{
	glBegin(type);
}

static void APIENTRY
tessVertex(GLvoid *vertex)
{
	const GLdouble *pointer;
	pointer = (GLdouble*)vertex;
	glVertex3dv(pointer);
}

static void APIENTRY
tessEnd()
{
	glEnd();
}

static void APIENTRY
tessError(GLenum errno)
{
	g_error("Tess Error: %i\n", errno);
}

static void APIENTRY
tessCombine(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], GLdouble **dataOut)
{
	//g_error("Tess Combine not implemented");
   GLdouble *vertex;
//   int i;

   vertex = (GLdouble*)calloc(3, sizeof(GLdouble)); //7
   vertex[0] = coords[0];
   vertex[1] = coords[1];
   vertex[2] = coords[2];
   /*vertex[3] = 0.0f; //r
   vertex[4] = 0.0f; //g
   vertex[5] = 0.0f; //b
   vertex[6] = 0.0f; //a
   for (i = 0; i < 4; i++)
   {
	   if (vertex_data[i])
	   {
		   vertex[3] += weight[0] * vertex_data[i][3];
		   vertex[4] += weight[1] * vertex_data[i][4];
		   vertex[5] += weight[2] * vertex_data[i][5];
		   vertex[6] += weight[3] * vertex_data[i][6];
	   }
   }*/

   *dataOut = vertex;

   lstMemCombine = g_slist_prepend(lstMemCombine, vertex);
}

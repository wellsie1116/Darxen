/* darxen.c

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

#define G_LOG_DOMAIN "Darxen"
#include <glib/gmessages.h>

#include "DarxenCommon.h"

#include "frmSplash.h"
#include "frmMain.h"
#include "DarxenParserLevel3.h"
#include "RadarViewer.h"
#include "DarxenConversions.h"

#include "libdarxenRadarSites.h"

#include <stdio.h>
#include <stdlib.h>

#include <glib/goption.h>
#include <glib/gutils.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkglinit.h>
#include <gtk/gtkhbox.h>

#include <shapefil.h>

#include <pthread.h>

static pthread_t thread;
static pthread_mutex_t mutex;
static pthread_cond_t cond;

static GOptionEntry entries[] =
{
/*	long name		short name	flags	argument type		variable		description												argument description */
	{NULL}
};

static void generate_shapefile_sites();

static void*
darxen_loadthread_run(void* data)
{
	printf("Hello from thread\n");fflush(stdout);
	pthread_exit(NULL);
	return NULL;
}

void
darxen_log(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
	FILE *out = fopen("log.txt", "a");

	if (!out)
	{
		printf("Warning:  Error opening log file for writing\n");
		return;
	}

	fprintf(out, "%s-%s: %s\n", log_domain, log_level == G_LOG_LEVEL_DEBUG ? "DEBUG" : log_level == G_LOG_LEVEL_MESSAGE ? "Message" : log_level == G_LOG_LEVEL_WARNING ? "WARNING **" : log_level == G_LOG_LEVEL_CRITICAL ? "CRITICAL **" : "ERROR **", message);
	fclose(out);
}

void exiting()
{
	if (0)
		generate_shapefile_sites();

	g_object_unref(G_OBJECT(settings_get_client()));

	printf("goodbye\n");
}

int main(int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;

	atexit(exiting);

	g_set_prgname("darxen");

	gtk_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);

	context = g_option_context_new("\nParse and render NEXRAD data");
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	/* g_option_context_set_ignore_unknown_options(context, TRUE); */
	if (!g_option_context_parse(context, &argc, &argv, &error))
	{
		printf("option parsing failed: %s\n", error->message);
		exit(1);
	}

    printf("darxen Copyright (C) 2011 Kevin Wells <kevin@darxen.org>\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY; for details type `%s -w'.\n", g_path_get_basename(argv[0]));
    printf("This is free software, and you are welcome to redistribute it\n");
    printf("under certain conditions; type `%s -c' for details.\n\n\n", g_path_get_basename(argv[0]));
	fflush(stdout);

	//Setup log handler
	remove("log.txt");
	//g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL, darxen_log, NULL);

	if (DEBUG)
	{
		int major;
		int minor;
		gdk_gl_query_version(&major, &minor);
		g_debug("OpenGL extension version - %i.%i", major, minor);
	}

	int rc;
	rc = pthread_create(&thread, NULL, darxen_loadthread_run, NULL);

	if (rc)
	{
		g_error("Error creating init thread: %i", rc);
	}

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);



	darxen_splash_run();

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	gtk_main();

	return 0;
}

static void
generate_shapefile_sites()
{
	GSList* pSites = darxen_radar_sites_get_site_list();

	SHPHandle hSHP;
	DBFHandle hDBF;

	hSHP = SHPCreate("radarSites", SHPT_POINT);
	hDBF = DBFCreate("radarSites");

	if (!hSHP || !hDBF)
		g_error("Failed to create sites shapefile");

	int fieldID = DBFAddField(hDBF, "ID", FTString, 4, 0);
	int fieldLat = DBFAddField(hDBF, "LAT", FTDouble, 8, 3);
	int fieldLon = DBFAddField(hDBF, "LON", FTDouble, 8, 3);
	int fieldState = DBFAddField(hDBF, "State", FTString, 3, 0);
	int fieldCity = DBFAddField(hDBF, "City", FTString, 20, 0);

	int partStart[] = {0};
	int partType[] = {SHPP_RING};

	while (pSites)
	{
		DarxenRadarSiteInfo* info = (DarxenRadarSiteInfo*)pSites->data;

		double x = info->fltLongitude;
		double y = info->fltLatitude;
		double zm = 0.0;

		SHPObject* object = SHPCreateObject(SHPT_POINT, -1, 1, partStart, partType, 1, &x, &y, &zm, &zm);

		int index = SHPWriteObject(hSHP, -1, object);

		SHPDestroyObject(object);

		DBFWriteStringAttribute(hDBF, index, fieldID, info->chrID);
		DBFWriteDoubleAttribute(hDBF, index, fieldLat, info->fltLatitude);
		DBFWriteDoubleAttribute(hDBF, index, fieldLon, info->fltLongitude);
		DBFWriteStringAttribute(hDBF, index, fieldState, info->chrState);
		DBFWriteStringAttribute(hDBF, index, fieldCity, info->chrCity);

		pSites = pSites->next;
	}
	DBFClose(hDBF);
	SHPClose(hSHP);
}


//void print_shapefile_info()
//{
//	if (FALSE)
//	{
//		DBFHandle psDBF;
//		int intFieldCount;
//		int i;
//		DBFFieldType intType;
//		char chrFieldName[20];
//
//		psDBF = DBFOpen("shapefiles\\ci22jl08.dbf", "rb");
//		intFieldCount = DBFGetFieldCount(psDBF);
//		printf("Fields: %i\n", intFieldCount);
//		for (i = 0; i < intFieldCount; i++)
//		{
//			intType = DBFGetFieldInfo(psDBF, i, chrFieldName, NULL, NULL);
//			printf("Field #%i:\n", i);
//			printf("Type: %s\n", intType == FTString ? "string" : intType == FTInteger ? "integer" : intType == FTDouble ? "double" : "???");
//			printf("Name: %s\n", chrFieldName);
//			printf("\n");
//		}
//		DBFClose(psDBF);
//	}
//
//	if (FALSE)
//	{
//		DBFHandle psDBF;
//		int intRecordCount;
//		int i;
//		int intMaxPop = 0;
//		int intPop;
//
//		psDBF = DBFOpen("shapefiles\\ci22jl08.dbf", "rb");
//		intRecordCount = DBFGetRecordCount(psDBF);
//		for (i = 0; i < intRecordCount; i++)
//		{
//			intPop = DBFReadIntegerAttribute(psDBF, i, 7);
//			if (intPop > intMaxPop)
//			{
//				printf("New max pop of %i reached by %s\n", intPop, DBFReadStringAttribute(psDBF, i, 9));
//				printf("BTW state=%s\n", DBFReadStringAttribute(psDBF, i, 10));
//				printf("BTW pop=%s\n", DBFReadStringAttribute(psDBF, i, 8));
//				printf("\n");
//				intMaxPop = intPop;
//			}
//		}
//		printf("Max pop: %i\n", intMaxPop);
//		printf("Name %s\n", DBFReadStringAttribute(psDBF, 4, 9));
//		printf("POP (1990): %i\n", DBFReadIntegerAttribute(psDBF, 4, 7));
//		DBFClose(psDBF);
//	}
//}






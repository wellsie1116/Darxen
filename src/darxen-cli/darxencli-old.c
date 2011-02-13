/* DarxenCLI.c

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

#include "DarxenCommon.h"

#define G_LOG_DOMAIN "DarxenCLI"
#include "glib/gmessages.h"

#include "DarxenConversions.h"
#include "DarxenRendererShared.h"
#include "DarxenRenderer.h"
#include "DarxenDataStructs.h"
#include "DarxenParserLevel3.h"
#include "DarxenSettings.h"

#include <glib.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkgl.h>

#include <png.h>

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

/* Option Variables  */
static gboolean blnWarranty		= FALSE;
static gboolean blnConditions	= FALSE;
static char *chrRenderFile		= NULL;
static char *chrLevel3File		= NULL;

void print_warranty();
void print_conditions();
int main_parse_level3(char *chrFile);
int main_render_level3(char *chrFile);

static GOptionEntry entries[] =
{
/*	long name		short name	flags	argument type		variable		description												argument description */
	{"warranty",			'w', 0, G_OPTION_ARG_NONE,		&blnWarranty,	"Show warranty information",							NULL},
	{"conditions",			'c', 0, G_OPTION_ARG_NONE,		&blnConditions,	"Show redistributing conditions",						NULL},
	{"render_data",			'r', 0, G_OPTION_ARG_STRING,	&chrRenderFile,	"Render a data file to the console",					"file"},
	{"print_level_3_data",	'3', 0, G_OPTION_ARG_STRING,	&chrLevel3File,	"Parse a level 3 data file",							"file"},
	{NULL}
};

int main(int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;

	context = g_option_context_new("\nParse and render NEXRAD data");
	g_option_context_add_main_entries(context, entries, NULL);
	/* g_option_context_set_ignore_unknown_options(context, TRUE); */
	if (!g_option_context_parse(context, &argc, &argv, &error))
	{
		printf("option parsing failed: %s\n", error->message);
		exit(1);
	}

	if (blnWarranty)
		print_warranty();
	else if (blnConditions)
		print_conditions();
	else if (chrRenderFile)
	{
		gtk_init (&argc, &argv);
		gtk_gl_init(&argc, &argv);
		return main_render_level3(chrRenderFile);
	}
	else if (chrLevel3File)
		return main_parse_level3(chrLevel3File);
	else
	{
		char *chrHelp = g_option_context_get_help(context, TRUE, NULL);
		printf("%s", chrHelp);
		g_free(chrHelp);
	}
	return 0;
}

void print_warranty()
{
	printf("  THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY\n");
	printf("APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT\n");
	printf("HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY\n");
	printf("OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,\n");
	printf("THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n");
	printf("PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM\n");
	printf("IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF\n");
	printf("ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n");
}

void print_conditions()
{
	printf("  4. Conveying Verbatim Copies.\n\n");

	printf("  You may convey verbatim copies of the Program's source code as you\n");
	printf("receive it, in any medium, provided that you conspicuously and\n");
	printf("appropriately publish on each copy an appropriate copyright notice;\n");
	printf("keep intact all notices stating that this License and any\n");
	printf("non-permissive terms added in accord with section 7 apply to the code;\n");
	printf("keep intact all notices of the absence of any warranty; and give all\n");
	printf("recipients a copy of this License along with the Program.\n\n");

	printf("  You may charge any price or no price for each copy that you convey,\n");
	printf("and you may offer support or warranty protection for a fee.\n\n");

	printf("  5. Conveying Modified Source Versions.\n\n");

	printf("  You may convey a work based on the Program, or the modifications to\n");
	printf("produce it from the Program, in the form of source code under the\n");
	printf("terms of section 4, provided that you also meet all of these conditions:\n\n");

	printf("    a) The work must carry prominent notices stating that you modified\n");
	printf("    it, and giving a relevant date.\n\n");

	printf("    b) The work must carry prominent notices stating that it is\n");
	printf("    released under this License and any conditions added under section\n");
	printf("    7.  This requirement modifies the requirement in section 4 to\n");
	printf("    \"keep intact all notices\".\n\n");

	printf("    c) You must license the entire work, as a whole, under this\n");
	printf("    License to anyone who comes into possession of a copy.  This\n");
	printf("    License will therefore apply, along with any applicable section 7\n");
	printf("    additional terms, to the whole of the work, and all its parts,\n");
	printf("    regardless of how they are packaged.  This License gives no\n");
	printf("    permission to license the work in any other way, but it does not\n");
	printf("    invalidate such permission if you have separately received it.\n\n");

	printf("    d) If the work has interactive user interfaces, each must display\n");
	printf("    Appropriate Legal Notices; however, if the Program has interactive\n");
	printf("    interfaces that do not display Appropriate Legal Notices, your\n");
	printf("    work need not make them do so.\n\n");

	printf("  A compilation of a covered work with other separate and independent\n");
	printf("works, which are not by their nature extensions of the covered work,\n");
	printf("and which are not combined with it such as to form a larger program,\n");
	printf("in or on a volume of a storage or distribution medium, is called an\n");
	printf("\"aggregate\" if the compilation and its resulting copyright are not\n");
	printf("used to limit the access or legal rights of the compilation's users\n");
	printf("beyond what the individual works permit.  Inclusion of a covered work\n");
	printf("in an aggregate does not cause this License to apply to the other\n");
	printf("parts of the aggregate.\n");
}

int main_parse_level3(char *chrFile)
{
	ProductsLevel3Data *objData = (ProductsLevel3Data*)malloc(sizeof(ProductsLevel3Data));
	ProductsLevel3MessageHeader *objMsgHead;
	ProductsLevel3Description *objDesc;
	ProductsLevel3SymbologyBlock *objSymBlock;
	ProductsLevel3GraphicAlphanumericBlock *objGraphAlphBlock;
	ProductsLevel3TabularAlphanumericBlock *objTabAlphBlock;
	FILE *objFile = NULL;
	int i;

	objFile = fopen(chrFile, "rb");
	if (ferror(objFile))
	{
		perror("Error opening the file");
		return 1;
	}
	if (!parser_lvl3_parse_file(objFile, objData))
	{
		printf("An error was encountered while parsing the file!\n");
		/*return 1;*/
	}

	/* get the data blocks */
	objMsgHead = &objData->objHeader;
	objDesc = &objData->objDescription;
	objSymBlock = objDesc->objSymbologyBlock;
	objGraphAlphBlock = objDesc->objGraphicAlphanumericBlock;
	objTabAlphBlock = objDesc->objTabularAlphanumericBlock;

	/* print WMO header */
	printf("Message WMO Header: %s\n\n", objData->chrWmoHeader);

	/* print header */
	printf("Message Header Information:\n");
	printf("Message Code: %i\n", objMsgHead->intMessageCode);
	printf("Message Date: "); darxen_print_date(objMsgHead->objDate); printf("\n");
	printf("Message Time: %i:%i:%i GMT\n", darxen_conversions_time_get_hours(objMsgHead->intTime), darxen_conversions_time_get_minutes(objMsgHead->intTime), darxen_conversions_time_get_seconds(objMsgHead->intTime));
	printf("Source ID: %i\n", objMsgHead->intSourceID);
	printf("Destination ID: %i\n", objMsgHead->intDestinationID);
	printf("Number of Blocks: %i\n\n", objMsgHead->intNumBlocks);

	/* print description */
	printf("Product Description:\n");
	printf("Latitude Position: %f\n", objDesc->fltLat);
	printf("Longitude Position: %f\n", objDesc->fltLon);
	printf("Altitude: %f\n", objDesc->fltAltitude);
	printf("Product Code: %i\n", objDesc->intProdCode);
	printf("Operation Mode: %i\n", objDesc->intOpMode);
	printf("Volume Coverage Pattern: %i\n", objDesc->intVolCovPat);
	printf("Sequence Number: %i\n", objDesc->intSeqNum);
	printf("Scan Number: %i\n", objDesc->intScanNum);
	printf("Scan Date: "); darxen_print_date(objDesc->objScanDate); printf("\n");
	printf("Scan Time: %i\n", objDesc->intScanTime);
	printf("Generation Date: "); darxen_print_date(objDesc->objGenDate); printf("\n");
	printf("Generation Time: %i\n", objDesc->intGenTime);
	printf("Product specific codes: ");
	for (i = 0; i < 9; i++)
		printf("%i,", objDesc->intProdCodes[i]);
	printf("%i\n", objDesc->intProdCodes[9]);
	printf("Elevation Number: %i\n", objDesc->intElevationNum);
	printf("Data Threshold Info:      ");
	for (i = 0; i < 15; i++)
		printf("%i,", objDesc->chrDataThresholdInfo[i]);
	printf("%i\n", objDesc->chrDataThresholdInfo[15]);
		printf("Data Threshold Value: ");
	for (i = 0; i < 15; i++)
		printf("%i,", objDesc->chrDataThresholdValue[i]);
	printf("%i\n", objDesc->chrDataThresholdValue[15]);
	printf("Version: %i\n", (int)objDesc->chrVersion);
	printf("Has Symbology Block: %s\n", objSymBlock == NULL ? "no" : "yes");
	printf("Has Graphic Block: %s\n", objGraphAlphBlock == NULL ? "no" : "yes");
	printf("Has Tabular Block: %s\n\n", objTabAlphBlock == NULL ? "no" : "yes");

	/* print symbology block */
	if (objDesc->objSymbologyBlock != NULL)
	{
		printf("Symbology Block:\n");
		printf("Number of Layers: %i\n", objSymBlock->intNumLayers);
		if (objSymBlock->intNumLayers > 0)
			printf("\n");

		/* print symbology block layers */
		for (i = 0; i < objSymBlock->intNumLayers; i++)
		{
			printf("Symbology Block Layer %i:\n", i);
			switch (objSymBlock->intPacketTypes[i])
			{
			case LEVEL3_RADIAL_DATA:
				{
					ProductsLevel3RadialDataPacket oRadDatPacket = objSymBlock->objPackets[i].objRadialData;
					printf("Radial Data Packet Information:\n");
					printf("Index of First Range Bin: %i\n", oRadDatPacket.intFirstRangeBin);
					printf("Number of Range Bins:     %i\n", oRadDatPacket.intNumRangeBins);
					printf("I Center of Sweep:        %i\n", oRadDatPacket.intCenterSweepI);
					printf("J Center of Sweep:        %i\n", oRadDatPacket.intCenterSweepJ);
					printf("Scale Factor:             %1.3f\n", oRadDatPacket.fltScaleFactor);
					printf("Number of Radials:        %i\n\n", oRadDatPacket.intRadialCount);
					/*
					for (x = 0; x < oRadDatPacket.intRadialCount; x++)
						printf("Radial #%i (%.1f + %.1f) Data (10th entry): %i\n", x, oRadDatPacket.objRadials[x].fltStartAngle, oRadDatPacket.objRadials[x].fltAngleDelta, oRadDatPacket.objRadials[x].chrColorCode[9]);
					printf("\n");
					*/
				}
				break;
			case LEVEL3_RASTER_DATA:
				printf("Error: Layer type not supported for console display\n");
				break;
			default:
				printf("Error: Layer type not supported for console display\n");
			} /* end switch */
			printf("\n");
		} /* end for */
	} /* end if */

	return 0;

} /*end main_parse_level3() */

int
main_render_level3(char *chrFile)
{
	FILE *objFile = NULL;
	ProductsLevel3Data objData;
	GdkGLConfig *glconfig = NULL;
	GdkPixmap *pixmap = NULL;
	GdkGLDrawable *gldrawable = NULL;
	GdkGLContext *glcontext = NULL;
	DarxenRenderer *renderer = NULL;
	SettingsView viewSettings;
	SettingsSite siteSettings;
	int i;
	unsigned char *pixels;

	printf("Opening file...");

	objFile = fopen(chrFile, "rb");
	if (!objFile)
		g_error("Could not open file");

	printf("done\nParsing file...");

	if (!parser_lvl3_parse_file(objFile, &objData))
		g_error("Could not parse file");

	fclose(objFile);

	printf("done\n");

	printf("done\ncreating gl config...");

	glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB | GDK_GL_MODE_SINGLE);
	if (!glconfig)
		g_error("Invalid configuration");

	printf("done\ncreating pixmap...");

	pixmap = gdk_pixmap_new(NULL, 800, 600, 16);
	if (!pixmap)
		g_error("Pixmap creation failed");

	printf("done\ncreating drawable...");

	gldrawable = GDK_GL_DRAWABLE(gdk_pixmap_set_gl_capability(pixmap, glconfig, NULL));
	if (!gldrawable)
		g_error("GL drawable creation failed");

	printf("done\ncreating context...");

	glcontext = gdk_gl_context_new(gldrawable, NULL, FALSE, GDK_GL_RGBA_TYPE);
	if (!glcontext)
		g_error("GL context creation failed");

	printf("done\nstarting draw...");

	if (!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		g_error("GL drawable draw failed");

	printf("done\n");

	darxen_renderer_shared_init_text();

	viewSettings.intDispListProductsBase = 0;

	for (i = 0; i < DISP_LIST_SITE_COUNT; i++)
		siteSettings.intDispLists[i] = 0;
	siteSettings.info = (SettingsRadarSiteInfo*)malloc(sizeof(SettingsRadarSiteInfo));
	strcpy(siteSettings.info->chrID, "KLOT");
	siteSettings.info->fltLatitude = (float)41.604;
	siteSettings.info->fltLongitude = (float)-88.085;
	

	renderer = DARXEN_RENDERER(darxen_renderer_new(&viewSettings, &siteSettings));

	darxen_renderer_set_size(renderer, 800, 600);
	darxen_renderer_set_data(renderer, &objData);

	darxen_renderer_render(renderer);

	pixels = (unsigned char*)malloc(sizeof(unsigned char*) * 800*600*3);
	glReadPixels(0, 0, 800, 600, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	objFile = fopen("pixels.txt", "wb");
	fwrite(pixels, 1, 800*600*3, objFile);
	fclose(objFile);

	gdk_gl_drawable_gl_end(gldrawable);
	
	g_object_unref(G_OBJECT(glcontext));
	g_object_unref(G_OBJECT(glconfig));
	return 0;
}

/* Settings.c

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

#define G_LOG_DOMAIN "DarxenSettings"
#include <glib/gmessages.h>

#include "DarxenSettings.h"

#define G_LOG_DOMAIN "DarxenSettings"
#include "glib/gmessages.h"

#include "libdarxenXml.h"
#include "libdarxenShapefiles.h"
#include "libdarxenFileSupport.h"

#include <shapefil.h>

#include <libxml/xpath.h>

typedef enum _DateTimeType	DateTimeType;

enum _DateTimeType
{
	DATE_TIME_NONE,
	DATE_TIME_START,
	DATE_TIME_END
};

/* Private Variables */
static xmlDoc*				docSettings			= NULL;
static xmlXPathContext*		xpathSettings		= NULL;

static SettingsGL*			mobjSettingsGL		= NULL;
static SettingsMain*		mobjSettingsMain	= NULL;

/* Private Functions */
static xmlXPathObject* evalXPath(const char* path);

static gboolean settings_load_main();

static SettingsPalette *settings_get_reflectivity_palette();
static SettingsPalette *settings_get_velocity_palette();
static SettingsPalette *settings_load_palette(char *chrPath);

/********************
 * Public Functions *
 ********************/

const SettingsGL*
settings_get_gl()
{
	return mobjSettingsGL;
}

void
settings_set_gl(SettingsGL *objSettingsGL)
{
	if (mobjSettingsGL)
		g_error("OpenGL settings are already set");

	mobjSettingsGL = objSettingsGL;
}

SettingsMain*
settings_get_main()
{
	if (!mobjSettingsMain)
	{
		mobjSettingsMain = (SettingsMain*)malloc(sizeof(SettingsMain));
		mobjSettingsMain->radview.intFrameStepInterval = 150;

		mobjSettingsMain->renderer.blnSmoothing = 0;
		mobjSettingsMain->renderer.fltOffsetX = 0.0f;
		mobjSettingsMain->renderer.fltOffsetY = 0.0f;
		mobjSettingsMain->renderer.fltScale = 1.0f;
		mobjSettingsMain->renderer.objPaletteReflectivity = settings_get_reflectivity_palette();
		mobjSettingsMain->renderer.objPaletteVelocity = settings_get_velocity_palette();
		if (!mobjSettingsMain->renderer.objPaletteReflectivity)
			g_error("Reflectivity palette failed to load\n");
		if (!mobjSettingsMain->renderer.objPaletteVelocity)
			g_error("Velocity palette failed to load\n");

		mobjSettingsMain->lstSites = NULL;
		mobjSettingsMain->lstAvailableRadarSites = NULL;

		if (settings_load_main())
		{
			GList *lstSite = mobjSettingsMain->lstSites;
			mobjSettingsMain->lstAvailableRadarSites = g_slist_copy(darxen_radar_sites_get_site_list());
			while (lstSite != NULL)
			{
				mobjSettingsMain->lstAvailableRadarSites = g_slist_remove(mobjSettingsMain->lstAvailableRadarSites, ((SettingsSite*)lstSite->data)->name);
				lstSite = lstSite->next;
			}
		}
		else
			g_error("Main settings failed to load (Where is Settings.xml?)\n");
	}

	return mobjSettingsMain;
}

SettingsRadarViewer*
settings_get_radar_viewer()
{
	return &settings_get_main()->radview;
}

static DarxenRestfulClient* client = NULL;

gboolean
settings_create_client(GError** error)
{
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	if (client)
		return TRUE;

	char* address;

	switch (settings_get_darxend_connection_type())
	{
	case DARXEND_CONNECTION_TYPE_LOCAL:
		//FIXME: implement
		client = NULL; //darxen_client_new("http://127.0.0.1:4888/DarxenService", settings_get_darxend_autostart());
		break;
	case DARXEND_CONNECTION_TYPE_REMOTE:
		address = settings_get_darxend_address();
		//FIXME: implement
		client = NULL; //darxen_client_new(address, FALSE);
		free(address);
		break;
	}

	//FIXME: implement
	if (1)//(!darxen_client_connect(client, error))
	{
		g_object_unref(G_OBJECT(client));
		client = NULL;
		return FALSE;
	}

	return TRUE;
}

DarxenRestfulClient*
settings_get_client()
{
	return client;
}

SettingsDarxendConnectionType
settings_get_darxend_connection_type()
{
	settings_get_main();

	char* type = settings_path_get_str("/Settings/Darxend/@type");
	SettingsDarxendConnectionType res = DARXEND_CONNECTION_TYPE_LOCAL;
	if (!strcmp(type, "local"))
		res = DARXEND_CONNECTION_TYPE_LOCAL;
	else if (!strcmp(type, "remote"))
		res = DARXEND_CONNECTION_TYPE_REMOTE;
	else
		g_warning("Invalid connection type (%s), using default: local", type);
	free(type);

	return res;
}

gboolean
settings_get_darxend_autostart()
{
	return settings_path_get_bool("/Settings/Darxend/Autostart/text()");
}

char*
settings_get_darxend_address()
{
	return settings_path_get_str("/Settings/Darxend/Address/text()");
}

void
settings_main_save()
{
	xmlTextWriter *writer;

	writer = xmlNewTextWriterFilename("settings2.xml", 0);
	xmlTextWriterStartDocument(writer, "1.0", "UTF-8", NULL);
	xmlTextWriterSetIndentString(writer, "\t");

	xmlTextWriterStartElement(writer, "Settings");
	xmlTextWriterWriteAttribute(writer, "version", "1.0");

	xmlTextWriterSetIndent(writer, 1);
	xmlTextWriterStartElement(writer, "RadarViewer");
		xmlTextWriterSetIndent(writer, 2);
		xmlTextWriterWriteElement(writer, "FrameStepInterval", "150");
		xmlTextWriterSetIndent(writer, 1);
	xmlTextWriterEndElement(writer);

	xmlTextWriterStartElement(writer, "Renderer");
		xmlTextWriterSetIndent(writer, 2);
		xmlTextWriterWriteElement(writer, "Smoothing", "0");
		xmlTextWriterWriteElement(writer, "Scale", "1.0");
		xmlTextWriterStartElement(writer, "Offset");
			xmlTextWriterSetIndent(writer, 3);
			xmlTextWriterWriteElement(writer, "X", "0.0");
			xmlTextWriterWriteElement(writer, "Y", "0.0");
			xmlTextWriterSetIndent(writer, 2);
		xmlTextWriterEndElement(writer);
		xmlTextWriterStartElement(writer, "Overlays");
			xmlTextWriterSetIndent(writer, 3);
			xmlTextWriterWriteElement(writer, "Counties", "1");
			xmlTextWriterWriteElement(writer, "States", "0");
			xmlTextWriterWriteElement(writer, "Cities", "0");
			xmlTextWriterWriteElement(writer, "Rivers", "0");
			xmlTextWriterWriteElement(writer, "Roads", "0");
			xmlTextWriterWriteElement(writer, "Lakes", "0");
			xmlTextWriterSetIndent(writer, 2);
		xmlTextWriterEndElement(writer);
		xmlTextWriterSetIndent(writer, 1);
	xmlTextWriterEndElement(writer);

	xmlTextWriterEndElement(writer);

	xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
}

void
settings_check_version()
{
	xmlDoc *doc = NULL;
	char *chrPath = g_build_filename(darxen_file_support_get_app_path(), "Settings.xml", NULL);
	float fltDefaultVersion;
	float fltSettingsVersion;

	if (g_file_test(chrPath, G_FILE_TEST_EXISTS))
		doc = xmlReadFile(chrPath, NULL, XML_PARSE_NOBLANKS);

	if (doc == NULL)
	{
		g_warning("Settings file nonexistant, loading defaults");
		settings_load_default();
	}
	else
	{
		char *chrVersion;

		chrVersion = xmlGetProp(xmlDocGetRootElement(doc), "version");
		if (!chrVersion)
		{
			fltSettingsVersion = 0.0f;
		}
		else
		{
			fltSettingsVersion = (float)atof(chrVersion);
			xmlFree(chrVersion);
		}
		xmlFreeDoc(doc);

		doc = xmlReadFile("Defaults.xml", NULL, XML_PARSE_NOBLANKS);
		if (!doc)
			g_error("Could not read default settings");

		chrVersion = xmlGetProp(xmlDocGetRootElement(doc), "version");
		if (!chrVersion)
		{
			fltDefaultVersion = 0.0f;
		}
		else
		{
			fltDefaultVersion = (float)atof(chrVersion);
			xmlFree(chrVersion);
		}
		xmlFreeDoc(doc);
		if (fltSettingsVersion < fltDefaultVersion)
		{
			g_warning("Settings file out of date, loading defaults");
			settings_load_default();
		}
	}
}

void
settings_load_default()
{
	char *chrPath = g_build_filename(darxen_file_support_get_app_path(), "Settings.xml", NULL);
	char *chrDefaults;

	if (!g_file_get_contents("Defaults.xml", &chrDefaults, NULL, NULL))
		g_error("Could not read default settings");
	if (!g_file_set_contents(chrPath, chrDefaults, -1, NULL))
		g_error("Could not write default settings");
	g_free(chrDefaults);
}

#define XPATH_FORMAT_PATH_EXPR(last) 	va_list args; \
										va_start(args, last); \
										gchar* expr = g_strdup_vprintf(path, args); \
										va_end(args);

#define XPATH_EVAL_EXPR			xmlXPathObject* xres; \
								xres = evalXPath(expr); \
								g_free(expr);

gboolean
settings_path_exists(char* path, ...)
{
	XPATH_FORMAT_PATH_EXPR(path)
	XPATH_EVAL_EXPR

	if (!xres)
		return FALSE;
	xmlXPathFreeObject(xres);
	return TRUE;
}

xmlNodeSet*
settings_path_get_nodes(const char* path, ...)
{
	XPATH_FORMAT_PATH_EXPR(path)
	xmlNodeSet* res = darxen_xml_get_nodes(xpathSettings, expr);
	g_free(expr);
	return res;
}

char**
settings_path_get_strs(char* path, int* count, ...)
{
	XPATH_FORMAT_PATH_EXPR(count)
	char** res = darxen_xml_get_strs(xpathSettings, expr, count);
	g_free(expr);
	return res;
}

GDate*
settings_path_get_date(const char* path, ...)
{
	XPATH_FORMAT_PATH_EXPR(path)
	GDate* res = darxen_xml_get_date(xpathSettings, expr);
	g_free(expr);
	return res;
}

int
settings_path_get_time(const char* path, ...)
{
	XPATH_FORMAT_PATH_EXPR(path)
	int res = darxen_xml_get_time(xpathSettings, expr);
	g_free(expr);
	return res;
}

char*
settings_path_get_str(char* path, ...)
{
	XPATH_FORMAT_PATH_EXPR(path)
	char* res = darxen_xml_get_string(xpathSettings, expr);
	g_free(expr);
	return res;
}

int
settings_path_get_int(char* path, ...)
{
	XPATH_FORMAT_PATH_EXPR(path)
	int res = darxen_xml_get_int(xpathSettings, expr);
	g_free(expr);
	return res;
}

double
settings_path_get_num(char* path, ...)
{
	XPATH_FORMAT_PATH_EXPR(path)
	double res = darxen_xml_get_number(xpathSettings, expr);
	g_free(expr);
	return res;
}

gboolean
settings_path_get_bool(char* path, ...)
{
	XPATH_FORMAT_PATH_EXPR(path)
	gboolean res = darxen_xml_get_boolean(xpathSettings, expr);
	g_free(expr);
	return res;
}

char*
settings_path_try_get_str(char* path, char* def_val, ...)
{
	XPATH_FORMAT_PATH_EXPR(def_val)
	char* res = darxen_xml_try_get_string(xpathSettings, def_val, expr);
	g_free(expr);
	return res;
}

int
settings_path_try_get_int(char* path, int def_val, ...)
{
	XPATH_FORMAT_PATH_EXPR(def_val)
	int res = darxen_xml_try_get_int(xpathSettings, def_val, expr);
	g_free(expr);
	return res;
}

double
settings_path_try_get_num(char* path, double def_val, ...)
{
	XPATH_FORMAT_PATH_EXPR(def_val)
	double res = darxen_xml_try_get_number(xpathSettings, def_val, expr);
	g_free(expr);
	return res;
}

gboolean
settings_path_try_get_bool(char* path, gboolean def_val, ...)
{
	XPATH_FORMAT_PATH_EXPR(def_val)
	gboolean res = darxen_xml_try_get_boolean(xpathSettings, def_val, expr);
	g_free(expr);
	return res;
}



void
settings_path_set_str(char* path, char* val, ...)
{
	XPATH_FORMAT_PATH_EXPR(val)
	XPATH_EVAL_EXPR

	g_assert(xres->type == XPATH_NODESET && xres->nodesetval && xres->nodesetval->nodeNr == 1);
	xmlNode* node = xres->nodesetval->nodeTab[0];
	xmlXPathFreeObject(xres);

	xmlNodeSetContent(node, val);
}

//FIXME: sets should create paths that don't exist
void
settings_path_set_int(char* path, int val, ...)
{
	XPATH_FORMAT_PATH_EXPR(val)
	gchar* new_val = g_strdup_printf("%i", val);

	settings_path_set_str(expr, new_val);
	g_free(expr);
}

void
settings_path_set_num(char* path, double val, ...)
{
	XPATH_FORMAT_PATH_EXPR(val)
	gchar* new_val = g_strdup_printf("%f", val);

	settings_path_set_str(expr, new_val);
	g_free(expr);
}

void
settings_path_set_boolean(char* path, gboolean val, ...)
{
	XPATH_FORMAT_PATH_EXPR(val)
	gchar* new_val = g_strdup_printf("%i", val);

	//FIXME: attributes should be true/false, text nodes should be 0/1/...

	settings_path_set_str(expr, new_val);
	g_free(expr);
}

void
settings_path_delete_path(char* path, ...)
{
	XPATH_FORMAT_PATH_EXPR(path)
	XPATH_EVAL_EXPR

	g_assert(xres->type == XPATH_NODESET && xres->nodesetval && xres->nodesetval->nodeNr == 1);

	xmlNode* node = xres->nodesetval->nodeTab[0];
	xmlXPathFreeObject(xres);

	xmlUnlinkNode(node);
	xmlFreeNode(node);
}

void
settings_path_swap_paths(char* path1, char* path2)
{
	xmlXPathObject* xres1 = evalXPath(path1);
	xmlXPathObject* xres2 = evalXPath(path2);

	g_assert(xres1->type == XPATH_NODESET && xres1->nodesetval && xres1->nodesetval->nodeNr == 1);
	g_assert(xres2->type == XPATH_NODESET && xres2->nodesetval && xres2->nodesetval->nodeNr == 1);

	xmlNode* node1 = xres1->nodesetval->nodeTab[0];
	xmlNode* node2 = xres2->nodesetval->nodeTab[0];
//	xmlNode* node1_clone = xmlCopyNode(node1, 0);
//	xmlNode* node2_clone = xmlCopyNode(node2, 0);

	//xmlFreeNode(xmlReplaceNode(node1, node2);
//	xmlReplaceNode

//	xmlNode* node = xres->nodesetval->nodeTab[0];
//	xmlXPathFreeObject(xres);
}


gchar*
settings_path_format(char* path, ...)
{
	XPATH_FORMAT_PATH_EXPR(path)
	return expr;
}



/*********************
 * Private Functions *
 *********************/

static xmlXPathObject*
evalXPath(const char* path)
{
	xmlXPathObject* res;
	res = xmlXPathEvalExpression(path, xpathSettings);

	if (!res)
	{
		g_error("XPath operation failed: %s", path);
	}

	return res;
}

static gboolean
settings_load_main()
{
	xmlDoc *doc;
	xmlXPathContext* xpath;
	char *filename;


	filename = darxen_file_support_get_overridable_file_path("Settings.xml");
	doc = xmlReadFile(filename, NULL, 0);

	if (!doc)
		return FALSE;

	xmlXPathInit();

	xpath = xmlXPathNewContext(doc);
	if (!xpath)
	{
		xmlFreeDoc(doc);
		return FALSE;
	}

	docSettings = doc;
	xpathSettings = xpath;

	//darxen_xml_debug(xpath, "/Settings/RadarViewer/FrameStepInterval");
	//darxen_xml_debug(xpath, "sum(/Settings/RadarViewer/FrameStepInterval)");
	//darxen_xml_debug(xpath, "count(/Settings/Sites/Site/@id)");
	//darxen_xml_debug(xpath, "/Settings/Sites/Site[1]/Views/View/Name");
	//darxen_xml_debug(xpath, "/Settings/Sites/Site[1]/@id");
	///darxen_xml_debug(xpath, "//@id/node()");
	//darxen_xml_debug(xpath, "/Settings");
	//darxen_xml_debug(xpath, "/Settings/Sites/Site[@id=\"klot\"]/Views/View[1]/Products/Product[1]/Smoothing/text()");
	//darxen_xml_debug(xpath, "/Settings/Sites/Site[@id=\"klot\"]/Views/View[1]/Products/Product[1]/Smoothing/text()");


	mobjSettingsMain->radview.intFrameStepInterval = (int)darxen_xml_get_number(xpath, "sum(/Settings/RadarViewer/FrameStepInterval)");
	mobjSettingsMain->lstSites = NULL;

	int siteCount = (int)darxen_xml_get_number(xpath, "count(/Settings/Sites/Site/@id)");
	int siteIndex;

	for (siteIndex = 1; siteIndex <= siteCount; siteIndex++)
	{
		SettingsSite* site = (SettingsSite*)malloc(sizeof(SettingsSite));
		mobjSettingsMain->lstSites = g_list_append(mobjSettingsMain->lstSites, site);

		site->name = settings_path_get_str("/Settings/Sites/Site[%i]/@id", siteIndex);
		site->views = NULL;

		int viewIndex;
		int viewCount = settings_path_get_int("count(/Settings/Sites/Site[@id=\"%s\"]/Views/View)", site->name);
		for (viewIndex = 1; viewIndex <= viewCount; viewIndex++)
		{
			SettingsView* view = (SettingsView*)malloc(sizeof(SettingsView));
			site->views = g_list_append(site->views, view);

			view->name = settings_path_get_str("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Name/text()", site->name, viewIndex);
			view->enabled = settings_path_get_bool("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Enabled/text()", site->name, viewIndex);
			char* style = settings_path_get_str("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Style/text()", site->name, viewIndex);
			if (!strcmp(style, "Single"))
			{
				view->viewstyle = VIEWSTYLE_SINGLE;
			}
			else if (!strcmp(style, "Quad"))
			{
				view->viewstyle = VIEWSTYLE_QUADRUPLE;
			}
			else
			{
				g_warning("Invalid viewstyle (%s), defaulting to single", style);
				view->viewstyle = VIEWSTYLE_SINGLE;
			}
			free(style);

			char* sourcetype = settings_path_get_str("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Source/@type", site->name, viewIndex);
			if (!strcmp(sourcetype, "Directory"))
			{
				view->source = VIEW_SOURCE_DIRECTORY;
				view->sourceParams.directory.path = settings_path_get_str("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Source/Path/text()", site->name, viewIndex);
			}
			else if (!strcmp(sourcetype, "Archives"))
			{
				view->source = VIEW_SOURCE_ARCHIVES;
				view->sourceParams.archive.dateStart = settings_path_get_date("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Source/DateTime[@type=\"start\"]/Date", site->name, viewIndex);
				view->sourceParams.archive.timeStart = settings_path_get_time("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Source/DateTime[@type=\"start\"]/Time", site->name, viewIndex);
				view->sourceParams.archive.dateEnd = settings_path_get_date("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Source/DateTime[@type=\"end\"]/Date", site->name, viewIndex);
				view->sourceParams.archive.timeEnd = settings_path_get_time("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Source/DateTime[@type=\"end\"]/Time", site->name, viewIndex);
			}
			else if (!strcmp(sourcetype, "Live"))
			{
				view->source = VIEW_SOURCE_LIVE;
			}
			else
			{
				g_error("Invalid viewsource (%s)", sourcetype);
			}
			free(sourcetype);

			view->products = NULL;

			int productCount = settings_path_get_int("count(/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Products/Product)", site->name, viewIndex);
			int productIndex;
			for (productIndex = 1; productIndex <= productCount; productIndex++)
			{
				SettingsViewProduct* product = (SettingsViewProduct*)malloc(sizeof(SettingsViewProduct));
				view->products = g_list_append(view->products, product);

				product->code = settings_path_get_str("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Products/Product[%i]/Code/text()", site->name, viewIndex, productIndex);
				product->smoothing = settings_path_get_int("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Products/Product[%i]/Smoothing/text()", site->name, viewIndex, productIndex);

				product->shapefiles = NULL;
				product->overlays = NULL;

				int shapefileCount = settings_path_get_int("count(/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Products/Product[%i]/Shapefiles/Shapefile)", site->name, viewIndex, productIndex);
				int shapefileIndex;
				for (shapefileIndex = 1; shapefileIndex <= shapefileCount; shapefileIndex++)
				{
					xmlNodeSet* nodes = settings_path_get_nodes("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Products/Product[%i]/Shapefiles/Shapefile[%i]", site->name, viewIndex, productIndex, shapefileIndex);
					g_assert(nodes->nodeNr == 1);

					char* id = settings_path_get_str("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Products/Product[%i]/Shapefiles/Shapefile[%i]/@id", site->name, viewIndex, productIndex, shapefileIndex);

					DarxenShapefile* shapefile = darxen_shapefiles_load_config(nodes->nodeTab[0], darxen_shapefiles_load_by_id(id));

					free(id);
					xmlXPathFreeNodeSet(nodes);

					product->shapefiles = g_list_append(product->shapefiles, shapefile);
				}

				int overlayCount = settings_path_get_int("count(/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Products/Product[%i]/Overlays/Overlay)", site->name, viewIndex, productIndex);
				int overlayIndex;
				for (overlayIndex = 1; overlayIndex <= overlayCount; overlayIndex++)
				{
					SettingsViewProductOverlay* overlay = (SettingsViewProductOverlay*)malloc(sizeof(SettingsViewProductOverlay));
					product->overlays = g_list_append(product->overlays, overlay);

					char* type = settings_path_get_str("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Products/Product[%i]/Overlays/Overlay[%i]/@type", site->name, viewIndex, productIndex, overlayIndex);
					overlay->type = OVERLAY_TYPE_LEGEND;
					if (!strcmp(type, "legend"))
						overlay->type = OVERLAY_TYPE_LEGEND;
					else if (!strcmp(type, "VCP"))
						overlay->type = OVERLAY_TYPE_VCP;
					else if (!strcmp(type, "MX"))
						overlay->type = OVERLAY_TYPE_MX;
					else if (!strcmp(type, "datetime"))
						overlay->type = OVERLAY_TYPE_DATETIME;
					else
						g_warning("Invalid overlay type (%s), defaulting to legend", type);
					free(type);

					char* location = settings_path_get_str("/Settings/Sites/Site[@id=\"%s\"]/Views/View[%i]/Products/Product[%i]/Overlays/Overlay[%i]/@location", site->name, viewIndex, productIndex, overlayIndex);
					overlay->location = OVERLAY_LOCATION_TOP_LEFT;
					if (!strcmp(location, "top-left"))
						overlay->location = OVERLAY_LOCATION_TOP_LEFT;
					else if (!strcmp(location, "top-right"))
						overlay->location = OVERLAY_LOCATION_TOP_RIGHT;
					else if (!strcmp(location, "bottom-left"))
						overlay->location = OVERLAY_LOCATION_BOTTOM_LEFT;
					else if (!strcmp(location, "bottom-right"))
						overlay->location = OVERLAY_LOCATION_BOTTOM_RIGHT;
					else
						g_warning("Invalid location (%s), defaulting to top-left", location);
					free(location);
				}
			}
		}
	}



//    xmlXPathFreeContext(xpath);
//    xmlFreeDoc(doc);

    return TRUE;
}

static SettingsPalette*
settings_get_reflectivity_palette()
{
	return settings_load_palette("Palettes/Reflectivity.xml");
}

static SettingsPalette*
settings_get_velocity_palette()
{
	SettingsPalette *objPalette;
	int i;

	objPalette = (SettingsPalette*)malloc(sizeof(SettingsPalette));

	for (i = 0; i < 16; i++)
	{
		objPalette->fltRed[i] = 1.0f;
		objPalette->fltGreen[i] = 1.0f;
		objPalette->fltBlue[i] = 1.0f;
		objPalette->fltAlpha[i] = 1.0f;
	}
	return objPalette;
}

static SettingsPalette*
settings_load_palette(char *chrPath)
{
	SettingsPalette *objPalette;
	xmlDoc *doc;
	xmlNode *cur;
	xmlNode *color;
	gint i = 0;
	char *filename;

	objPalette = (SettingsPalette*)malloc(sizeof(SettingsPalette));

	filename = darxen_file_support_get_overridable_file_path(chrPath);
	if (!filename)
		g_error("Palette (%s) was not found", chrPath);
	doc = xmlReadFile(filename, NULL, XML_PARSE_NOBLANKS);

	if (doc == NULL)
		return FALSE;

//	xmlXPathContext* xpath;
//	xmlXPathObject* res;
//
//	xpath = xmlXPathNewContext(doc);
//	res = xmlXPathEvalExpression("/Palette/Color", xpath);



	cur = xmlDocGetRootElement(doc);
	cur = cur->children;

	while (cur != NULL)
	{
		objPalette->fltRed[i]	= 1.0f;
		objPalette->fltGreen[i]	= 1.0f;
		objPalette->fltBlue[i]	= 1.0f;
		objPalette->fltAlpha[i]	= 1.0f;
		color = cur->children;
		while (color != NULL)
		{
			if (!xmlStrcmp(color->name, "R"))
				objPalette->fltRed[i] = (float)atoi(xmlNodeListGetString(doc, color->children, 1)) / 255;
			else if (!xmlStrcmp(color->name, "G"))
				objPalette->fltGreen[i] = (float)atoi(xmlNodeListGetString(doc, color->children, 1)) / 255;
			else if (!xmlStrcmp(color->name, "B"))
				objPalette->fltBlue[i] = (float)atoi(xmlNodeListGetString(doc, color->children, 1)) / 255;
			else if (!xmlStrcmp(color->name, "A"))
				objPalette->fltAlpha[i] = (float)atoi(xmlNodeListGetString(doc, color->children, 1)) / 255;
			color = color->next;
		}
		cur = cur->next;
		i++;
	}
	xmlFreeDoc(doc);
	return objPalette;
}


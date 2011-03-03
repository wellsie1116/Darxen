/* Settings.h

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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "DarxenCommon.h"

#include "libdarxenRadarSites.h"
#include "libdarxenRestfulClient.h"

#include <string.h>
#include <gdk/gdkglconfig.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

#include <libxml/xpath.h>

G_BEGIN_DECLS

typedef enum _SettingsViewStyle					SettingsViewStyle;
typedef enum _SettingsViewSource				SettingsViewSource;
typedef enum _SettingsOverlay					SettingsOverlay;
typedef enum _DarxenDisplayListsSite			DarxenDisplayListsSite;
typedef enum _SettingsOverlayType				SettingsOverlayType;
typedef enum _SettingsOverlayLocation			SettingsOverlayLocation;
typedef enum _SettingsDarxendConnectionType		SettingsDarxendConnectionType;

typedef struct _SettingsColor					SettingsColor;
typedef struct _SettingsPalette					SettingsPalette;
typedef struct _SettingsRenderer				SettingsRenderer;
typedef struct _SettingsRadarViewer				SettingsRadarViewer;
typedef struct _SettingsShapefileTextLabels		SettingsShapefileTextLabels;
typedef struct _SettingsShapefilePoints			SettingsShapefilePoints;
typedef struct _SettingsShapefileLines			SettingsShapefileLines;
typedef struct _SettingsShapefilePolygons		SettingsShapefilePolygons;
typedef struct _SettingsViewProductOverlay		SettingsViewProductOverlay;
typedef struct _SettingsViewProduct				SettingsViewProduct;
typedef struct _SettingsView					SettingsView;
typedef struct _SettingsViewSourceDirectory		SettingsViewSourceDirectory;
typedef struct _SettingsViewSourceArchive		SettingsViewSourceArchive;
typedef union  _SettingsViewSourceParameters	SettingsViewSourceParameters;
typedef struct _SettingsSite					SettingsSite;
typedef struct _SettingsGL						SettingsGL;//
typedef struct _SettingsMain					SettingsMain;

enum _SettingsDarxendConnectionType
{
	DARXEND_CONNECTION_TYPE_LOCAL,
	DARXEND_CONNECTION_TYPE_REMOTE
};

enum _SettingsViewStyle
{
	VIEWSTYLE_SINGLE,
	VIEWSTYLE_QUADRUPLE
};

enum _SettingsViewSource
{
	VIEW_SOURCE_LIVE,
	VIEW_SOURCE_ARCHIVES,
	VIEW_SOURCE_DIRECTORY
};

enum _SettingsOverlay
{
	OVERLAY_SITES_POINTS,
	OVERLAY_SITES_LABELS,
	OVERLAY_COUNTIES_OUTLINE,
	OVERLAY_STATES_OUTLINE,
	OVERLAY_CITIES_POINTS,
	OVERLAY_CITIES_LABELS,
	OVERLAY_RIVERS,
	OVERLAY_ROADS,
	OVERLAY_LAKES,

	OVERLAY_COUNT
};

enum _DarxenDisplayListsSite
{
	DISP_LIST_SITE_SITES_DOTS,
	DISP_LIST_SITE_SITES_LABELS,
	DISP_LIST_SITE_COUNTIES,
	DISP_LIST_SITE_STATES,
	DISP_LIST_SITE_CITIES_DOTS,
	DISP_LIST_SITE_CITIES_LABELS,
	DISP_LIST_SITE_RIVERS,
	DISP_LIST_SITE_LAKES,
	DISP_LIST_SITE_ROADS,

	DISP_LIST_SITE_COUNT
};

enum _SettingsOverlayType
{
	OVERLAY_TYPE_LEGEND,
	OVERLAY_TYPE_VCP,
	OVERLAY_TYPE_MX,
	OVERLAY_TYPE_DATETIME,

	OVERLAY_TYPE_COUNT
};

enum _SettingsOverlayLocation
{
	OVERLAY_LOCATION_TOP_LEFT,
	OVERLAY_LOCATION_TOP_RIGHT,
	OVERLAY_LOCATION_BOTTOM_LEFT,
	OVERLAY_LOCATION_BOTTOM_RIGHT
};

struct _SettingsColor
{
	float r;
	float g;
	float b;
	float a;
};

struct _SettingsPalette
{
	float fltRed[16];
	float fltGreen[16];
	float fltBlue[16];
	float fltAlpha[16];
};

struct _SettingsRenderer
{
	//TODO: expunge this struct from existence

	/* Settings.xml */
	gboolean blnSmoothing;

	/* Non-XML */
	float fltOffsetX;
	float fltOffsetY;
	float fltScale;

	SettingsPalette *objPaletteReflectivity;
	SettingsPalette *objPaletteVelocity;
};


struct _SettingsRadarViewer
{
	/* Settings.xml */
	int intFrameStepInterval;
};

struct _SettingsViewSourceDirectory
{
	char* path;
};

struct _SettingsViewSourceArchive
{
	GDate* dateStart;
	GDate* dateEnd;
	int timeStart;
	int timeEnd;
};

union _SettingsViewSourceParameters
{
	SettingsViewSourceDirectory directory;
	SettingsViewSourceArchive archive;
};


struct _SettingsViewProductOverlay
{
	SettingsOverlayType type;
	SettingsOverlayLocation location;
};

struct _SettingsViewProduct
{
	char* code;
	gboolean smoothing;
	GList* shapefiles; /* DarxenShapefile */
	GList* overlays; /* SettingsViewProductOverlay */
};

struct _SettingsView
{
	/* Settings.xml */
	char *name;
	gboolean enabled;
	SettingsViewStyle viewstyle;
	SettingsViewSource source;
	SettingsViewSourceParameters sourceParams;
	GList *products; /* SettingsViewProduct */

	/* Non-XML */
	float fltOffsetX;
	float fltOffsetY;
	float fltScale;

	gpointer radview;
	guint intDispListProductsBase;
};



struct _SettingsSite
{
	/* Settings.xml */
	char *name;
	GList *views; /* SettingsView */
};

struct _SettingsGL
{
	/* Non-XML */
	GdkGLContext *glShareList;
	GdkGLConfig *glConfig;
};

struct _SettingsMain
{
	/* Settings.xml */
	SettingsRadarViewer radview;
	SettingsRenderer renderer;
	GList *lstSites; /* SettingsSite */

	/* Non-XML */
	GSList *lstAvailableRadarSites; /* RadarSiteInfo */
};

const SettingsGL*		settings_get_gl();
void					settings_set_gl(SettingsGL *objSettingsGL);
SettingsMain*			settings_get_main();
SettingsRadarViewer*	settings_get_radar_viewer();

//char*					settings_get_palette_path(char* product, int vcp)

gboolean				settings_create_client(GError** error);
DarxenRestfulClient*			settings_get_client();

SettingsDarxendConnectionType	settings_get_darxend_connection_type();
gboolean						settings_get_darxend_autostart();
char*							settings_get_darxend_address();

void settings_main_save();

void settings_check_version();
void settings_load_default();

/*
#define PATH_COUNT_OF(xpath)			"count(" xpath ")"

#define PATH_SITE_NAMES					"/Settings/Sites/Site/@id"

#define PATH_$SITE			 			"/Settings/Sites/Site[@id=\"%s\"]"
#define PATH_VIEW_NAMES 				"/Views/View/Name/text()"

#define PATH_$SITE_VIEW_COUNT			PATH_COUNT_OF(PATH_$SITE "/Views/View")
#define PATH_$SITE_$VIEW				PATH_$SITE "/Views/View[%i]"

#define PATH_NAME						"/Name"
#define PATH_ENABLED					"/Enabled"
#define PATH_STYLE						"/Style"

#define PATH_$SITE_$VIEW_NAME			PATH_$SITE_$VIEW PATH_NAME
#define PATH_$SITE_$VIEW_ENABLED		PATH_$SITE_$VIEW PATH_ENABLED
#define PATH_$SITE_$VIEW_STYLE			PATH_$SITE_$VIEW PATH_STYLE

#define PATH_$SITE_VIEW_NAMES 			PATH_SITE PATH_VIEW_NAMES



#define settings_get_sites_count()							settings_path_get_int("count(/Settings/Sites/Site)")
#define settings_get_site_id(iSite)							settings_path_get_str("/Settings/Site/Site[%i]/@id", iSite)
#define settings_get_site_view_count(iSite)					settings_path_get_int("count(/Settings/Site/Site[%i]/Views/View)", iSite)
#define settings_get_site_view_name(iSite, iView)			settings_path_get_str("/Settings/Site/Site[%i]/Views/View[%i]/Name", iSite, iView)
#define settings_get_site_view_enabled(iSite, iView)		settings_path_get_bool("/Settings/Site/Site[%i]/Views/View[%i]/Enabled", iSite, iView)
#define settings_get_site_view_source_type(iSite, iView)	settings_path_get_str("/Settings/Site/Site[%i]/Views/View[%i]/Source/@type", iSite, iView)*/
/*
 * Settings file
 *
 * Loaded once, any changes made while application is running will be overwritten
 *
 * Saved whenever a mutation occurs (set/swap/delete) using the DOM document stored
 * in memory.
 */


//new path interface
gboolean	settings_path_exists(char* path, ...);

xmlNodeSet*	settings_path_get_nodes(const char* path, ...);

char**		settings_path_get_strs(char* path, int* count, ...);
GDate*		settings_path_get_date(const char* path, ...);
int			settings_path_get_time(const char* path, ...);

char*		settings_path_get_str(char* path, ...);
int			settings_path_get_int(char* path, ...);
double		settings_path_get_num(char* path, ...);
gboolean	settings_path_get_bool(char* path, ...);

char*		settings_path_try_get_str(char* path, char* def_val, ...);
int			settings_path_try_get_int(char* path, int def_val, ...);
double		settings_path_try_get_num(char* path, double def_val, ...);
gboolean 	settings_path_try_get_bool(char* path, gboolean def_val, ...);

void		settings_path_set_str(char* path, char* val, ...);
void		settings_path_set_int(char* path, int val, ...);
void		settings_path_set_num(char* path, double val, ...);
void		settings_path_set_boolean(char* path, gboolean val, ...);

void		settings_path_delete_path(char* path, ...);
void		settings_path_swap_paths(char* path1, char* path2);

gchar*		settings_path_format(char* path, ...);

G_END_DECLS

#endif

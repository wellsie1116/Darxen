

#ifndef LIBDARXENRADARSITES_H_
#define LIBDARXENRADARSITES_H_

#include "libdarxenCommon.h"

#include <glib/gslist.h>

typedef struct _DarxenRadarSiteInfo DarxenRadarSiteInfo;

struct _DarxenRadarSiteInfo
{
	/* Sites.xml */
	char chrID[5];
	float fltLatitude;
	float fltLongitude;
	char chrState[3];
	char chrCity[20];
};

G_EXPORT void						darxen_radar_sites_init();
G_EXPORT DarxenRadarSiteInfo*		darxen_radar_sites_get_site_info(const char* id);
G_EXPORT GSList*					darxen_radar_sites_get_site_list(); /* DarxenRadarSiteInfo */
G_EXPORT GSList*					darxen_radar_sites_get_site_name_list(); /* char */

#endif /*LIBDARXENRADARSITES_H_*/


#include "libdarxenRadarSites.h"

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#define CHECK_INIT	if (!init) darxen_radar_sites_init();

static gboolean init		= FALSE;
static GSList* lstSites		= NULL;
static GSList* lstSiteNames	= NULL;


/******************
 * Public Methods *
 ******************/

void
darxen_radar_sites_init()
{
	xmlDoc *doc;
	xmlNode *cur;
	xmlNode *site;
	DarxenRadarSiteInfo *info;
	gint i;

	doc = xmlReadFile("Sites.xml", NULL, XML_PARSE_NOBLANKS);

	g_assert(doc);

	cur = xmlDocGetRootElement(doc);
	cur = cur->children;

	while (cur != NULL)
	{
		site = cur->children;
		info = (DarxenRadarSiteInfo*)malloc(sizeof(DarxenRadarSiteInfo));
		while (site != NULL)
		{
			if (!xmlStrcmp(site->name, "ID"))
				strcpy(info->chrID, xmlNodeListGetString(doc, site->children, 1));
			else if (!xmlStrcmp(site->name, "Lat"))
				info->fltLatitude = (float)strtod(xmlNodeListGetString(doc, site->children, 1), NULL);
			else if (!xmlStrcmp(site->name, "Lon"))
				info->fltLongitude = (float)strtod(xmlNodeListGetString(doc, site->children, 1), NULL);
			else if (!xmlStrcmp(site->name, "State"))
				strcpy(info->chrState, xmlNodeListGetString(doc, site->children, 1));
			else if (!xmlStrcmp(site->name, "City"))
				strcpy(info->chrCity, xmlNodeListGetString(doc, site->children, 1));
			site = site->next;
		}
		for (i = 0; i < 4; i++)
			if ((int)info->chrID[i] > 96 && (int)info->chrID[i] < 123)
				info->chrID[i] = (unsigned char)((int)info->chrID[i] - 32);
		lstSites = g_slist_append(lstSites, info);
		lstSiteNames = g_slist_append(lstSiteNames, strdup(info->chrID));
		cur = cur->next;
	}
	xmlFreeDoc(doc);

	init = TRUE;
}


DarxenRadarSiteInfo*
darxen_radar_sites_get_site_info(const char* id)
{
	CHECK_INIT

	GSList* pSites = lstSites;
	while (pSites)
	{
		if (!strcasecmp(((DarxenRadarSiteInfo*)pSites->data)->chrID, id))
			return (DarxenRadarSiteInfo*)pSites->data;
		pSites = pSites->next;
	}
	return NULL;
}


GSList*
darxen_radar_sites_get_site_list()
{
	CHECK_INIT

	return lstSites;
}

GSList*
darxen_radar_sites_get_site_name_list()
{
	CHECK_INIT

	return lstSiteNames;
}


/*******************
 * Private Methods *
 *******************/



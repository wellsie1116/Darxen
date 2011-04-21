/* darxenmainview.c
 *
 * Copyright (C) 2011 - Kevin Wells <kevin@darxen.org>
 *
 * This file is part of darxen
 *
 * darxen is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * darxen is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with darxen.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "darxenmainview.h"
#include "darxensitelist.h"
#include "darxenconfig.h"

#include <libdarxenRadarSites.h>

#include <glib.h>

static GltkWidget* root = NULL;
static GltkWidget* sites;

void
darxen_main_view_readd_site(const gchar* name)
{
	DarxenRadarSiteInfo* siteInfo = darxen_radar_sites_get_site_info(name);

	gchar* display = g_strdup_printf("%s, %s", siteInfo->chrCity, siteInfo->chrState);
	GltkWidget* lblSite = gltk_label_new(display);
	g_free(display);

	gltk_list_add_item(GLTK_LIST(sites), lblSite, siteInfo);
}

static GltkWidget*
sites_convert_dropped_item(GltkList* list, const gchar* targetType, GltkListItem* item, gpointer user_data)
{
	gchar* name = darxen_site_list_free_site_data(item->data);

	DarxenRadarSiteInfo* siteInfo = darxen_radar_sites_get_site_info(name);
	g_free(name);

	gchar* display = g_strdup_printf("%s, %s", siteInfo->chrCity, siteInfo->chrState);
	GltkWidget* lblSite = gltk_label_new(display);
	g_free(display);

	item->data = siteInfo;

	return lblSite;
}

static gboolean
check_site_list(const gchar* id)
{
	GList* pSites = darxen_config_get_sites(darxen_config_get_instance());
	while (pSites)
	{
		DarxenSiteInfo* siteInfo = (DarxenSiteInfo*)pSites->data;
	
		if (!g_ascii_strcasecmp(id, siteInfo->name))
			return TRUE;
	
		pSites = pSites->next;
	}
	return FALSE;
}

GltkWidget*
darxen_main_view_get_root()
{
	if (root)
		return root;

	root = gltk_vbox_new(0);
	g_object_ref_sink(G_OBJECT(root));

	GltkWidget* sitesScrollable = gltk_scrollable_new();
	{
		sites = gltk_list_new();
		g_object_set(sites, "target-type", "SiteList", NULL);

		GSList* lstSites = darxen_radar_sites_get_site_list();
		GSList* plstSites = lstSites;
		while (plstSites)
		{
			DarxenRadarSiteInfo* siteInfo = (DarxenRadarSiteInfo*)plstSites->data;

			if (check_site_list(siteInfo->chrID))
			{
				plstSites = plstSites->next;
				continue;
			}

			gchar* display = g_strdup_printf("%s, %s", siteInfo->chrCity, siteInfo->chrState);
			GltkWidget* lblSite = gltk_label_new(display);
			g_free(display);
		
			gltk_list_add_item(GLTK_LIST(sites), lblSite, siteInfo);
		
			plstSites = plstSites->next;
		}
		g_signal_connect(G_OBJECT(sites), "convert-dropped-item", G_CALLBACK(sites_convert_dropped_item), NULL);
		GltkWidget* hboxSites = gltk_hbox_new(0);
		gltk_box_append_widget(GLTK_BOX(hboxSites), sites, FALSE, FALSE);
		gltk_scrollable_set_widget(GLTK_SCROLLABLE(sitesScrollable), hboxSites);
	}

	gltk_box_append_widget(GLTK_BOX(root), gltk_label_new("Welcome to Darxen Tablet Edition"), FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(root), sitesScrollable, TRUE, TRUE);

	return root;
}


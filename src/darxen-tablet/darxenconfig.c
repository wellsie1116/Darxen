/* darxenconfig.c
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

#include "darxenconfig.h"

GList* sites = NULL;

GList*
darxen_config_get_sites()
{
	if (!sites)
	{
		DarxenSiteInfo* site;
		DarxenViewInfo* view;

		//KLOT
		site = g_new(DarxenSiteInfo, 1);
		site->name = "KLOT";
		site->views = NULL;

		view = g_new(DarxenViewInfo, 1);
		view->name = "Archived Reflectivity (0.5)";
		view->productCode = g_strdup("N0R");
		view->sourceType = DARXEN_VIEW_SOURCE_ARCHIVE;
		view->source.archive.startId = g_strdup("200801030402");
		view->source.archive.endId = g_strdup("200901030402");
		site->views = g_list_append(site->views, view);

		view = g_new(DarxenViewInfo, 1);
		view->name = "Archived Reflectivity (1.0)";
		view->productCode = g_strdup("N1R");
		view->sourceType = DARXEN_VIEW_SOURCE_ARCHIVE;
		view->source.archive.startId = g_strdup("200801030402");
		view->source.archive.endId = g_strdup("200901030402");
		site->views = g_list_append(site->views, view);

		sites = g_list_append(sites, site);

		//KIND
		site = g_new(DarxenSiteInfo, 1);
		site->name = "KIND";
		site->views = NULL;

		view = g_new(DarxenViewInfo, 1);
		view->name = "Archived Reflectivity (0.5)";
		view->productCode = g_strdup("N0R");
		view->sourceType = DARXEN_VIEW_SOURCE_ARCHIVE;
		view->source.archive.startId = g_strdup("200801030402");
		view->source.archive.endId = g_strdup("200901030402");
		site->views = g_list_append(site->views, view);

		view = g_new(DarxenViewInfo, 1);
		view->name = "Archived Reflectivity (1.0)";
		view->productCode = g_strdup("N1R");
		view->sourceType = DARXEN_VIEW_SOURCE_ARCHIVE;
		view->source.archive.startId = g_strdup("200801030402");
		view->source.archive.endId = g_strdup("200901030402");
		site->views = g_list_append(site->views, view);

		sites = g_list_append(sites, site);
	}

	return sites;
}


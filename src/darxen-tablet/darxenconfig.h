/* darxenconfig.h
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

#ifndef DARXENCONFIG_H_Y4AZDBSX
#define DARXENCONFIG_H_Y4AZDBSX

#include <libdarxenRestfulClient.h>

#include <glib.h>

typedef struct	_DarxenSiteInfo				DarxenSiteInfo;
typedef struct	_DarxenViewSourceArchive	DarxenViewSourceArchive;
typedef enum	_DarxenViewSourceType		DarxenViewSourceType;
typedef union	_DarxenViewSource			DarxenViewSource;
typedef struct	_DarxenViewInfo				DarxenViewInfo;

struct _DarxenSiteInfo
{
	gchar* name;
	GList* views;
	GHashTable* viewMap;
};

struct _DarxenViewSourceArchive
{
	gchar* startId;
	gchar* endId;
};

enum _DarxenViewSourceType
{
	DARXEN_VIEW_SOURCE_ARCHIVE
};

union _DarxenViewSource
{
	DarxenViewSourceArchive archive;
};

struct _DarxenViewInfo
{
	gchar* name;
	gchar* productCode;

	DarxenViewSourceType sourceType;
	DarxenViewSource source;

	gboolean smoothing;
};


GList* darxen_config_get_sites();

void					darxen_config_set_client(DarxenRestfulClient* newClient);
DarxenRestfulClient*	darxen_config_get_client();

#endif

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

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define DARXEN_CONFIG_ERROR darxen_config_error_quark()

#define DARXEN_TYPE_CONFIG				(darxen_config_get_type())
#define DARXEN_CONFIG(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_CONFIG, DarxenConfig))
#define DARXEN_CONFIG_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_CONFIG, DarxenConfigClass))
#define DARXEN_IS_CONFIG(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_CONFIG))
#define DARXEN_IS_CONFIG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_CONFIG))
#define DARXEN_CONFIG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_CONFIG, DarxenConfigClass))

typedef struct	_DarxenSiteInfo				DarxenSiteInfo;
typedef struct	_DarxenViewSourceArchive	DarxenViewSourceArchive;
typedef enum	_DarxenViewSourceType		DarxenViewSourceType;
typedef union	_DarxenViewSource			DarxenViewSource;
typedef struct	_DarxenViewInfo				DarxenViewInfo;
typedef struct	_DarxenConfig				DarxenConfig;
typedef struct	_DarxenConfigClass			DarxenConfigClass;

struct _DarxenSiteInfo
{
	gchar* name;
	GList* views; //DarxenViewInfo
	//GHashTable* viewMap;
};

struct _DarxenViewSourceArchive
{
	gchar* startId;
	gchar* endId;
};

enum _DarxenViewSourceType
{
	DARXEN_VIEW_SOURCE_ARCHIVE,
	DARXEN_VIEW_SOURCE_LIVE
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

	GSList* shapefiles; //DarxenShapefile

	gboolean smoothing;
};

struct _DarxenConfig
{
	GObject parent;

	DarxenRestfulClient* client;
	GList* sites;
};

struct _DarxenConfigClass
{
	GObjectClass parent_class;
	
	/* signals */
	void (*site_added)			(	DarxenConfig* config,
									const gchar* site,
									int index);

	void (*site_deleted)		(	DarxenConfig* config,
									const gchar* site);

	void (*view_deleted)		(	DarxenConfig* config,
									const gchar* site,
									const gchar* view);

	void (*view_name_changed)	(	DarxenConfig* config,
									gchar* site,
									DarxenViewInfo* viewInfo,
									gchar* oldName);

	void (*view_updated)		(	DarxenConfig* config,
									gchar* site,
									gchar* viewName,
									DarxenViewInfo* viewInfo);

	/* virtual funcs */
};

typedef enum
{
	DARXEN_CONFIG_ERROR_FAILED
} DarxenConfigError;

GType					darxen_config_get_type			(	) G_GNUC_CONST;
DarxenConfig*			darxen_config_get_instance		(	);

void					darxen_config_load_settings		(	DarxenConfig* config);
void					darxen_config_save_settings		(	DarxenConfig* config);

GList*					darxen_config_get_sites			(	DarxenConfig* config);

void					darxen_config_set_client		(	DarxenConfig* config, 
															DarxenRestfulClient* newClient);

DarxenRestfulClient*	darxen_config_get_client		(	DarxenConfig* config);

void					darxen_config_add_site			(	DarxenConfig* config,
															const gchar* site,
															int index);

void					darxen_config_move_site			(	DarxenConfig* config,
															const gchar* site,
															int oldIndex,
															int newIndex);

void					darxen_config_delete_site		(	DarxenConfig* config,
															const gchar* site);

gboolean				darxen_config_rename_view		(	DarxenConfig* config,
															const gchar* site,
															const DarxenViewInfo* viewInfo,
															const gchar* newName);

void					darxen_config_view_updated		(	DarxenConfig* config,
															const gchar* site,
															const gchar* viewName,
															DarxenViewInfo* viewInfo);
															
DarxenViewInfo*			darxen_view_info_copy			(	const DarxenViewInfo* viewInfo);
void					darxen_view_info_free			(	DarxenViewInfo* viewInfo);


GQuark					darxen_config_error_quark		();

G_END_DECLS

#endif

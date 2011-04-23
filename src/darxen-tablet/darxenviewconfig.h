/* darxenviewconfig.h
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

#ifndef DARXENVIEWCONFIG_H_MYSW6MOR
#define DARXENVIEWCONFIG_H_MYSW6MOR

#include <gltk/gltk.h>

#include "darxenconfig.h"

G_BEGIN_DECLS

#define DARXEN_VIEW_CONFIG_ERROR darxen_view_config_error_quark()

#define DARXEN_TYPE_VIEW_CONFIG				(darxen_view_config_get_type())
#define DARXEN_VIEW_CONFIG(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_VIEW_CONFIG, DarxenViewConfig))
#define DARXEN_VIEW_CONFIG_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_VIEW_CONFIG, DarxenViewConfigClass))
#define DARXEN_IS_VIEW_CONFIG(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_VIEW_CONFIG))
#define DARXEN_IS_VIEW_CONFIG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_VIEW_CONFIG))
#define DARXEN_VIEW_CONFIG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_VIEW_CONFIG, DarxenViewConfigClass))

typedef struct _DarxenViewConfig			DarxenViewConfig;
typedef struct _DarxenViewConfigClass		DarxenViewConfigClass;

struct _DarxenViewConfig
{
	GltkTable parent;
};

struct _DarxenViewConfigClass
{
	GltkTableClass parent_class;
	
	/* signals */
	void	(*site_changed)	(	);

	/* virtual funcs */
};

typedef enum
{
	DARXEN_VIEW_CONFIG_ERROR_FAILED
} DarxenViewConfigError;

GType				darxen_view_config_get_type	() G_GNUC_CONST;
GltkWidget*			darxen_view_config_new		(gchar* site, DarxenViewInfo* viewInfo);

void				darxen_view_config_save		(DarxenViewConfig* viewConfig);
void				darxen_view_config_revert	(DarxenViewConfig* viewConfig);

GQuark			darxen_view_config_error_quark	();

G_END_DECLS

#endif


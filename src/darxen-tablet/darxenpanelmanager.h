/* darxenpanelmanager.h
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

#ifndef DARXENPANELMANAGER_H_L9L8GCYX
#define DARXENPANELMANAGER_H_L9L8GCYX

#include <gltk/gltk.h>

#include "darxenconfig.h"

G_BEGIN_DECLS

#define DARXEN_PANEL_MANAGER_ERROR darxen_panel_manager_error_quark()

#define DARXEN_TYPE_PANEL_MANAGER				(darxen_panel_manager_get_type())
#define DARXEN_PANEL_MANAGER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_PANEL_MANAGER, DarxenPanelManager))
#define DARXEN_PANEL_MANAGER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_PANEL_MANAGER, DarxenPanelManagerClass))
#define DARXEN_IS_PANEL_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_PANEL_MANAGER))
#define DARXEN_IS_PANEL_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_PANEL_MANAGER))
#define DARXEN_PANEL_MANAGER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_PANEL_MANAGER, DarxenPanelManagerClass))

typedef struct _DarxenPanelManager			DarxenPanelManager;
typedef struct _DarxenPanelManagerClass		DarxenPanelManagerClass;

struct _DarxenPanelManager
{
	GltkBin parent;
};

struct _DarxenPanelManagerClass
{
	GltkBinClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	DARXEN_PANEL_MANAGER_ERROR_FAILED
} DarxenPanelManagerError;

GType		darxen_panel_manager_get_type			() G_GNUC_CONST;
GltkWidget*	darxen_panel_manager_new				();

void		darxen_panel_manager_create_view		(DarxenPanelManager* manager, gchar* site, DarxenViewInfo* viewInfo);
void		darxen_panel_manager_destroy_view		(DarxenPanelManager* manager, gchar* site, gchar* view);

void		darxen_panel_manager_view_main			(DarxenPanelManager* manager);
void		darxen_panel_manager_view_view			(DarxenPanelManager* manager, gchar* site, gchar* view);
void		darxen_panel_manager_view_view_config	(DarxenPanelManager* manager, gchar* site, gchar* view);
void		darxen_panel_manager_save_view_config	(DarxenPanelManager* manager, gchar* site, gchar* view);
void		darxen_panel_manager_revert_view_config	(DarxenPanelManager* manager, gchar* site, gchar* view);

GQuark		darxen_panel_manager_error_quark	();

G_END_DECLS

#endif


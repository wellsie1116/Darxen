/* darxensitelist.h
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

#ifndef DARXENSITELIST_H_LS064R9N
#define DARXENSITELIST_H_LS064R9N

#include <glib-object.h>

#include <gltk/gltk.h>

G_BEGIN_DECLS

#define DARXEN_SITE_LIST_ERROR darxen_site_list_error_quark()

#define DARXEN_TYPE_SITE_LIST				(darxen_site_list_get_type())
#define DARXEN_SITE_LIST(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_SITE_LIST, DarxenSiteList))
#define DARXEN_SITE_LIST_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_SITE_LIST, DarxenSiteListClass))
#define DARXEN_IS_SITE_LIST(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_SITE_LIST))
#define DARXEN_IS_SITE_LIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_SITE_LIST))
#define DARXEN_SITE_LIST_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_SITE_LIST, DarxenSiteListClass))

typedef struct _DarxenSiteList			DarxenSiteList;
typedef struct _DarxenSiteListClass		DarxenSiteListClass;

struct _DarxenSiteList
{
	GltkList parent;
};

struct _DarxenSiteListClass
{
	GltkListClass parent_class;
	
	/* signals */

	void (*view_selected)	(	gchar* site,
								gchar* view);
	/* virtual funcs */
};

typedef enum
{
	DARXEN_SITE_LIST_ERROR_FAILED
} DarxenSiteListError;

GType			darxen_site_list_get_type	() G_GNUC_CONST;
GltkWidget*		darxen_site_list_new		();

void			darxen_site_list_add_site	(DarxenSiteList* list, const gchar* site);
void			darxen_site_list_add_view	(DarxenSiteList* list, const gchar* site, const gchar* view);

GQuark			darxen_site_list_error_quark	();

G_END_DECLS

#endif


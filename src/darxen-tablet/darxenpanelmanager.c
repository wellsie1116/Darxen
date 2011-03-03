/* darxenpanelmanager.c
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

#include "darxenpanelmanager.h"

#include "darxenmainview.h"
#include "darxenview.h"

G_DEFINE_TYPE(DarxenPanelManager, darxen_panel_manager, GLTK_TYPE_BIN)

#define USING_PRIVATE(obj) DarxenPanelManagerPrivate* priv = DARXEN_PANEL_MANAGER_GET_PRIVATE(obj)
#define DARXEN_PANEL_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_PANEL_MANAGER, DarxenPanelManagerPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _DarxenPanelManagerPrivate		DarxenPanelManagerPrivate;
typedef struct _SiteViewPair					SiteViewPair;

struct _DarxenPanelManagerPrivate
{
	GHashTable* viewMap;
	GltkWidget* mainView;
};

struct _SiteViewPair
{
	gchar* site;
	gchar* view;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void darxen_panel_manager_dispose(GObject* gobject);
static void darxen_panel_manager_finalize(GObject* gobject);

static SiteViewPair*	site_view_pair_new		(const gchar* site, const gchar* view);
static void				site_view_pair_free		(SiteViewPair* pair);
static guint			site_view_pair_hash		(SiteViewPair* pair);
static gboolean			site_view_pair_equal	(const SiteViewPair* o1, const SiteViewPair* o2);
static void				view_free				(DarxenView* view);

static void
darxen_panel_manager_class_init(DarxenPanelManagerClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(DarxenPanelManagerPrivate));
	
	gobject_class->dispose = darxen_panel_manager_dispose;
	gobject_class->finalize = darxen_panel_manager_finalize;
}

static void
darxen_panel_manager_init(DarxenPanelManager* self)
{
	USING_PRIVATE(self);

	priv->viewMap = NULL;
	priv->mainView = darxen_main_view_get_root();
}

static void
darxen_panel_manager_dispose(GObject* gobject)
{
	DarxenPanelManager* self = DARXEN_PANEL_MANAGER(gobject);
	USING_PRIVATE(self);

	//free and release references

	G_OBJECT_CLASS(darxen_panel_manager_parent_class)->dispose(gobject);
}

static void
darxen_panel_manager_finalize(GObject* gobject)
{
	DarxenPanelManager* self = DARXEN_PANEL_MANAGER(gobject);
	USING_PRIVATE(self);

	//free memory

	G_OBJECT_CLASS(darxen_panel_manager_parent_class)->finalize(gobject);
}

GltkWidget*
darxen_panel_manager_new()
{
	GObject *gobject = g_object_new(DARXEN_TYPE_PANEL_MANAGER, NULL);
	DarxenPanelManager* self = DARXEN_PANEL_MANAGER(gobject);

	USING_PRIVATE(self);

	priv->viewMap = g_hash_table_new_full((GHashFunc)site_view_pair_hash, (GEqualFunc)site_view_pair_equal, (GDestroyNotify)site_view_pair_free, (GDestroyNotify)view_free);

	darxen_panel_manager_view_main(self);

	return (GltkWidget*)gobject;
}

void
darxen_panel_manager_create_view(DarxenPanelManager* manager, gchar* site, DarxenViewInfo* viewInfo)
{
	g_return_if_fail(DARXEN_IS_PANEL_MANAGER(manager));
	g_return_if_fail(site);
	g_return_if_fail(viewInfo);
	USING_PRIVATE(manager);

	DarxenView* view = (DarxenView*)darxen_view_new(viewInfo);
	g_object_ref(G_OBJECT(view));

	g_hash_table_insert(priv->viewMap, site_view_pair_new(site, viewInfo->name), view);
}

void
darxen_panel_manager_view_main(DarxenPanelManager* manager)
{
	g_return_if_fail(DARXEN_IS_PANEL_MANAGER(manager));
	USING_PRIVATE(manager);

	gltk_bin_set_widget(GLTK_BIN(manager), priv->mainView);
}

void
darxen_panel_manager_view_view(DarxenPanelManager* manager, gchar* site, gchar* view)
{
	g_return_if_fail(DARXEN_IS_PANEL_MANAGER(manager));
	g_return_if_fail(site);
	g_return_if_fail(view);
	USING_PRIVATE(manager);

	DarxenView* widget = g_hash_table_lookup(priv->viewMap, site_view_pair_new(site, view));
	g_return_if_fail(GLTK_IS_WIDGET(widget));

	gltk_bin_set_widget(GLTK_BIN(manager), (GltkWidget*)widget);
}

void
darxen_panel_manager_view_view_config(DarxenPanelManager* manager, gchar* site, gchar* view)
{
	g_return_if_fail(DARXEN_IS_PANEL_MANAGER(manager));
	g_return_if_fail(site);
	g_return_if_fail(view);
	//USING_PRIVATE(manager);

	g_critical("Not implemented");
}

GQuark
darxen_panel_manager_error_quark()
{
	return g_quark_from_static_string("darxen-panel-manager-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static SiteViewPair*
site_view_pair_new(const gchar* site, const gchar* view)
{
	SiteViewPair* pair = g_new(SiteViewPair, 1);
	pair->site = g_strdup(site);
	pair->view = g_strdup(view);
	return pair;
}

static void
site_view_pair_free(SiteViewPair* pair)
{
	g_free(pair->site);
	g_free(pair->view);
	g_free(pair);
}

static guint
site_view_pair_hash(SiteViewPair* pair)
{
	return g_str_hash(pair->site) * 13 + g_str_hash(pair->view);
}

static gboolean
site_view_pair_equal(const SiteViewPair* o1, const SiteViewPair* o2)
{
	return g_str_equal(o1->site, o2->site) && g_str_equal(o1->view, o2->view);
}

static void
view_free(DarxenView* view)
{
	g_object_unref(G_OBJECT(view));
}


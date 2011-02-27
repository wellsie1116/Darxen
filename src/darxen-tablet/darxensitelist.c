/* darxensitelist.c
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

#include "darxensitelist.h"

#include <glib.h>

G_DEFINE_TYPE(DarxenSiteList, darxen_site_list, GLTK_TYPE_VBOX)

#define USING_PRIVATE(obj) DarxenSiteListPrivate* priv = DARXEN_SITE_LIST_GET_PRIVATE(obj)
#define DARXEN_SITE_LIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_SITE_LIST, DarxenSiteListPrivate))

enum
{
	//new view selected
	//sites reordered
	//views reordered
	//view deleted
	LAST_SIGNAL
};

typedef struct _DarxenSiteListPrivate		DarxenSiteListPrivate;
typedef struct _SiteViewInfo				SiteViewInfo;
typedef struct _Site						Site;
typedef struct _View						View;

struct _DarxenSiteListPrivate
{
	GList* sites; //gchar*
	GHashTable* siteMap; //Site

	Site* dragSite;
	GList* dragViewNode;
};

struct _SiteViewInfo
{
	DarxenSiteList* list;
	gchar* site;
	gchar* view;
};

struct _Site
{
	gchar* name;
	GltkWidget* siteBox;
	GltkWidget* viewsBox;
	GList* views; //gchar*
	GHashTable* viewMap; //View
};

struct _View
{
	gchar* name;
	SiteViewInfo info;
	GltkWidget* viewButton;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void darxen_site_list_dispose(GObject* gobject);
static void darxen_site_list_finalize(GObject* gobject);

static gboolean	site_button_clicked(GltkButton* button, GltkEventClick* event, Site* siteInfo);
static gboolean view_button_long_touch(GltkButton* button, GltkEventClick* event, SiteViewInfo* info);
static gboolean view_button_drag(GltkButton* button, GltkEventDrag* event, SiteViewInfo* info);
// static void darxen_site_list_size_request(GltkWidget* widget, GltkSize* size);
static void darxen_site_list_render(GltkWidget* widget);

static void
darxen_site_list_class_init(DarxenSiteListClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(DarxenSiteListPrivate));
	
	gobject_class->dispose = darxen_site_list_dispose;
	gobject_class->finalize = darxen_site_list_finalize;

	// gltkwidget_class->size_request = darxen_site_list_size_request;
	gltkwidget_class->render = darxen_site_list_render;
}

static void
site_free(Site* site)
{
	g_free(site->name);
	g_object_unref(G_OBJECT(site->viewsBox));
	g_object_unref(G_OBJECT(site->siteBox));
	g_hash_table_destroy(site->viewMap);
	GList* pView;
	for (pView = site->views; pView; pView = pView->next)
		g_free(pView->data);
	g_list_free(site->views);
	g_free(site);
}

static void
view_free(View* view)
{
	g_free(view->name);
	g_object_unref(G_OBJECT(view->viewButton));
	g_free(view);
}

static void
darxen_site_list_init(DarxenSiteList* self)
{
	USING_PRIVATE(self);

	priv->sites = NULL;
	priv->siteMap = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)site_free);
	priv->dragSite = NULL;
	priv->dragViewNode = NULL;
}

static void
darxen_site_list_dispose(GObject* gobject)
{
	DarxenSiteList* self = DARXEN_SITE_LIST(gobject);
	USING_PRIVATE(self);

	if (priv->siteMap)
	{
		g_hash_table_destroy(priv->siteMap);
		priv->siteMap = NULL;
	}

	if (priv->sites)
	{
		GList* pSite;
		for (pSite = priv->sites; pSite; pSite = pSite->next)
			g_free(pSite->data);
		g_list_free(priv->sites);
		priv->sites = NULL;
	}

	G_OBJECT_CLASS(darxen_site_list_parent_class)->dispose(gobject);
}

static void
darxen_site_list_finalize(GObject* gobject)
{
	DarxenSiteList* self = DARXEN_SITE_LIST(gobject);
	USING_PRIVATE(self);

	//free memory

	G_OBJECT_CLASS(darxen_site_list_parent_class)->finalize(gobject);
}

GltkWidget*
darxen_site_list_new()
{
	GObject *gobject = g_object_new(DARXEN_TYPE_SITE_LIST, NULL);
	DarxenSiteList* self = DARXEN_SITE_LIST(gobject);

	USING_PRIVATE(self);

	return (GltkWidget*)gobject;
}

void
darxen_site_list_add_site(DarxenSiteList* list, const gchar* site)
{
	g_return_if_fail(DARXEN_IS_SITE_LIST(list));
	USING_PRIVATE(list);

	GList* siteNode;
	siteNode = g_list_find_custom(priv->sites, site, (GCompareFunc)g_strcmp0);
	g_return_if_fail(!siteNode);

	Site* siteInfo = g_new(Site, 1);
	siteInfo->name = g_strdup(site);
	siteInfo->siteBox = gltk_vbox_new();
	siteInfo->views = NULL;
	siteInfo->viewMap = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)view_free);

	GltkWidget* siteButton = gltk_button_new(site);
	g_signal_connect(siteButton, "click_event", (GCallback)site_button_clicked, siteInfo);
	gltk_box_append_widget(GLTK_BOX(siteInfo->siteBox), siteButton, FALSE, FALSE);
	g_object_unref(G_OBJECT(siteButton));

	GltkWidget* viewSpacer = gltk_hbox_new();
	siteInfo->viewsBox = gltk_vbox_new();
	GltkWidget* spacer = gltk_label_new("  ");
	gltk_box_append_widget(GLTK_BOX(viewSpacer), spacer, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(viewSpacer), siteInfo->viewsBox, TRUE, TRUE);
	g_object_unref(G_OBJECT(spacer));

	gltk_box_append_widget(GLTK_BOX(siteInfo->siteBox), viewSpacer, FALSE, FALSE);
	g_object_unref(G_OBJECT(viewSpacer));

	gltk_box_append_widget(GLTK_BOX(list), siteInfo->siteBox, FALSE, FALSE);

	priv->sites = g_list_append(priv->sites, g_strdup(site));
	g_hash_table_insert(priv->siteMap, g_strdup(site), siteInfo);
}

void
darxen_site_list_add_view(DarxenSiteList* list, const gchar* site, const gchar* view)
{
	g_return_if_fail(DARXEN_IS_SITE_LIST(list));
	USING_PRIVATE(list);

	Site* siteInfo = g_hash_table_lookup(priv->siteMap, site);
	g_return_if_fail(siteInfo);

	GList* viewNode;
	viewNode = g_list_find_custom(siteInfo->views, view, (GCompareFunc)g_strcmp0);
	g_return_if_fail(!viewNode);

	View* viewInfo = g_new(View, 1);
	viewInfo->name = g_strdup(view);
	viewInfo->info.list = list;
	viewInfo->info.site = siteInfo->name;
	viewInfo->info.view = viewInfo->name;
	viewInfo->viewButton = gltk_button_new(view);
	g_signal_connect(viewInfo->viewButton, "long_touch_event", (GCallback)view_button_long_touch, &viewInfo->info);
	g_signal_connect(viewInfo->viewButton, "drag_event", (GCallback)view_button_drag, &viewInfo->info);

	gltk_box_append_widget(GLTK_BOX(siteInfo->viewsBox), viewInfo->viewButton, FALSE, FALSE);

	siteInfo->views = g_list_append(siteInfo->views, g_strdup(view));
	g_hash_table_insert(siteInfo->viewMap, g_strdup(view), viewInfo);
}


GQuark
darxen_site_list_error_quark()
{
	return g_quark_from_static_string("darxen-site-list-error-quark");
}

/*********************
 * Private Functions *
 *********************/
static gboolean
site_button_clicked(GltkButton* button, GltkEventClick* event, Site* siteInfo)
{
	g_message("Button clicked");

	return TRUE;
}

static gboolean
view_button_long_touch(GltkButton* button, GltkEventClick* event, SiteViewInfo* info)
{
	g_message("View long touched");

	USING_PRIVATE(info->list);

	Site* siteInfo = g_hash_table_lookup(priv->siteMap, info->site);
	g_assert(siteInfo);

	View* viewInfo = g_hash_table_lookup(siteInfo->viewMap, info->view);
	g_assert(viewInfo);
	g_assert(viewInfo->viewButton == GLTK_WIDGET(button));

	priv->dragSite = siteInfo;
	priv->dragViewNode = g_list_find_custom(siteInfo->views, info->view, (GCompareFunc)g_strcmp0);
	//priv->dragButtonParentAllocation = gltk_allocation_translate_to_global(gltk_widget_get_parent(priv->dragButton));
	gltk_box_remove_widget(GLTK_BOX(siteInfo->viewsBox), viewInfo->viewButton);
	gltk_widget_set_parent(viewInfo->viewButton, GLTK_WIDGET(info->list));

	gltk_widget_invalidate(GLTK_WIDGET(info->list));
	gltk_widget_layout(GLTK_WIDGET(info->list));

	return TRUE;
}

static GList*
g_list_swap(GList* list, GList* a, GList* b)
{
	g_return_val_if_fail(a && b && a != b, list);

	if (a->prev == b)
	{
		GList* tmp = a;
		a = b;
		b = tmp;
	}

	if (a->next == b)
	{
		a->next = b->next;
		if (a->next)
			a->next->prev = a;

		b->prev = a->prev;
		if (b->prev)
			b->prev->next = b;

		b->next = a;
		a->prev = b;

		if (a == list)
			return b;
		return list;
	}
	else
	{
		g_error("You broke it");
	}
}

static gboolean
view_button_drag(GltkButton* button, GltkEventDrag* event, SiteViewInfo* info)
{
	g_message("Button Drag");

	USING_PRIVATE(info->list);

	if (!priv->dragSite)
		return FALSE;

	View* viewInfo = (View*)g_hash_table_lookup(priv->dragSite->viewMap, (gchar*)priv->dragViewNode->data);

	GltkAllocation allocation = gltk_widget_get_allocation(viewInfo->viewButton);
	//allocation.x += event->dx;
	allocation.y += event->dy;
	gltk_widget_size_allocate(viewInfo->viewButton, allocation);
	gltk_widget_invalidate(viewInfo->viewButton);

	if (priv->dragViewNode->prev)
	{
		GList* prev = priv->dragViewNode->prev;
		View* prevViewInfo = (View*)g_hash_table_lookup(priv->dragSite->viewMap, (gchar*)prev->data);
		GltkAllocation o = gltk_widget_get_allocation(prevViewInfo->viewButton);
		if (allocation.y <= o.y)
		{
			priv->dragSite->views = g_list_swap(priv->dragSite->views, prev, priv->dragViewNode);
		}
	}


	return TRUE;
}

// static void
// darxen_site_list_size_request(GltkWidget* widget, GltkSize* size)
// {
// 	size->width = 300;
// 	size->height = 400;
// 
// 	GLTK_WIDGET_CLASS(darxen_site_list_parent_class)->size_request(widget, size);
// }
// 
static void
darxen_site_list_render(GltkWidget* widget)
{
	DarxenSiteList* list = DARXEN_SITE_LIST(widget);
	USING_PRIVATE(list);

	if (priv->dragSite)
	{
		glPushMatrix();
		{
			View* viewInfo = (View*)g_hash_table_lookup(priv->dragSite->viewMap, (gchar*)priv->dragViewNode->data);
			GltkAllocation allocation = gltk_widget_get_allocation(viewInfo->viewButton);
			GltkAllocation parentAllocation = gltk_allocation_translate_to_global(priv->dragSite->viewsBox);
			glTranslatef(parentAllocation.x + allocation.x, parentAllocation.y + allocation.y, 0.5f);
			gltk_widget_render(viewInfo->viewButton);
		}
		glPopMatrix();
	}

	GLTK_WIDGET_CLASS(darxen_site_list_parent_class)->render(widget);
}


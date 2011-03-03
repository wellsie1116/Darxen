/* frmMain.c

   Copyright (C) 2011 Kevin Wells <kevin@darxen.org>

   This file is part of darxen.

   darxen is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   darxen is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with darxen.  If not, see <http://www.gnu.org/licenses/>.
*/

#define G_LOG_DOMAIN "frmMain"
#include <glib/gmessages.h>

#include "frmMain.h"

#include <glib/gslist.h>
#include <glib/glist.h>

/* Private Variables */
static GtkWidget *frmMain;
static GtkWidget *radviewCurrent;
static GtkWidget *ntbkSites;
static GtkWidget *trvwAvailableSites;
static GtkWidget *trvwCurrentSites;

static GtkWidget *btnFrameFirst;
static GtkWidget *btnFramePrevious;
static GtkWidget *btnFramePlay;
static GtkWidget *btnFrameNext;
static GtkWidget *btnFrameLast;

static GtkListStore* mlstSites;
static GtkTreeModel* mlstAvailableSites;
static GtkTreeModel* mlstCurrentSites;

const GtkTargetEntry siteTargets[] = {{"text/csv", 0, 0}};

/* Events */
G_EXPORT gint	on_frmMain_delete_event	(GtkWidget *widget);
G_EXPORT void	on_frmMain_destroy		(GObject *widget);

G_EXPORT void	on_btnFrameFirst_clicked	(GtkWidget *widget);
G_EXPORT void	on_btnFramePrevious_clicked	(GtkWidget *widget);
G_EXPORT void	on_btnFramePlay_clicked		(GtkWidget *widget);
G_EXPORT void	on_btnFrameNext_clicked		(GtkWidget *widget);
G_EXPORT void	on_btnFrameLast_clicked		(GtkWidget *widget);

G_EXPORT void	on_btnCurrentSiteMoveUp_clicked		(GtkWidget *widget);
G_EXPORT void	on_btnCurrentSiteMoveDown_clicked	(GtkWidget *widget);
G_EXPORT void	on_btnSiteAdd_clicked				(GtkWidget *widget);
G_EXPORT void	on_btnSiteRemove_clicked			(GtkWidget *widget);

G_EXPORT void	on_mnuQuit_activate		(GtkWidget *widget);
G_EXPORT void	on_mnuAbout_activate	(GtkWidget *widget);

void			on_current_sites_row_inserted		(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data);
void			on_current_sites_row_deleted		(GtkTreeModel *tree_model, GtkTreePath *path, gpointer user_data);
void			on_current_sites_rows_reordered		(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, gpointer new_order, gpointer user_data);

G_EXPORT void	on_tree_view_sites_drag_data_get		(GtkWidget *widget, GdkDragContext *dc, GtkSelectionData *selection_data, guint info, guint t, gpointer data);
G_EXPORT void	on_tree_view_sites_drag_data_received	(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer data);
G_EXPORT void	on_tree_view_sites_drag_data_delete		(GtkWidget *widget, GdkDragContext *dc, gpointer data);

void			on_radar_viewer_frame_changed	(DarxenNotebookProducts *notebook_products, DarxenRadarViewer *radview);
void			on_ntbkProducts_page_changed	(DarxenNotebookProducts *ntbk, DarxenRadarSiteInfo *site, DarxenRadarViewer *radview);

/* Private Functions */
void	update_frame_sensitivity();


/********************
 * Public Functions *
 ********************/

enum ModelSitesColumn
{
	MODEL_SITES_ID,
	MODEL_SITES_STATE,
	MODEL_SITES_CITY,
	MODEL_SITES_CURRENT,
	MODEL_SITES_COUNT
};

gboolean is_not_current(GtkTreeModel* model, GtkTreeIter* iter, gpointer data)
{
	gboolean current;
	gtk_tree_model_get(model, iter, MODEL_SITES_CURRENT, &current, -1);
	return !current;
}

void
a()
{
	GtkListStore* model = gtk_list_store_new(MODEL_SITES_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
	GtkTreeIter iter;

	gtk_list_store_append(model, &iter);

	gtk_list_store_set(model, &iter,
			MODEL_SITES_ID, "klot",
			MODEL_SITES_STATE, "IN",
			MODEL_SITES_CITY, "Dyer",
			MODEL_SITES_CURRENT, TRUE,
			-1);

	gtk_list_store_set(model, &iter,
			MODEL_SITES_ID, "kind",
			MODEL_SITES_STATE, "IN",
			MODEL_SITES_CITY, "Indianapolis",
			MODEL_SITES_CURRENT, FALSE,
			-1);

	GtkTreeModel* filtered = gtk_tree_model_filter_new(GTK_TREE_MODEL(model), NULL);
	gtk_tree_model_filter_set_visible_column(GTK_TREE_MODEL_FILTER(filtered), MODEL_SITES_CURRENT);

	GtkTreeModel* filtered_inv = gtk_tree_model_filter_new(GTK_TREE_MODEL(model), NULL);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(filtered_inv), is_not_current, NULL, NULL);
}


void
darxen_main_new(GtkWindow* main, GtkBuilder* builder)
{
	SettingsMain *objSettingsMain;
	GList *lstSites;
	GSList* lstAvailableSites;
	GtkWidget *ntbkProducts;
	GtkListStore *list_store;
	GtkTreeIter iter;
	DarxenRadarSiteInfo *info;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	radviewCurrent = NULL;

	frmMain =				(GtkWidget*)gtk_builder_get_object(builder, "frmMain");
	btnFrameFirst =			(GtkWidget*)gtk_builder_get_object(builder, "btnFrameFirst");
	btnFramePrevious =		(GtkWidget*)gtk_builder_get_object(builder, "btnFramePrevious");
	btnFramePlay =			(GtkWidget*)gtk_builder_get_object(builder, "btnFramePlay");
	btnFrameNext =			(GtkWidget*)gtk_builder_get_object(builder, "btnFrameNext");
	btnFrameLast =			(GtkWidget*)gtk_builder_get_object(builder, "btnFrameLast");
	ntbkSites =				(GtkWidget*)gtk_builder_get_object(builder, "ntbkSites");
	trvwAvailableSites =	(GtkWidget*)gtk_builder_get_object(builder, "trvwAvailableSites");
	trvwCurrentSites =		(GtkWidget*)gtk_builder_get_object(builder, "trvwCurrentSites");

	objSettingsMain = settings_get_main();

	mlstSites = gtk_list_store_new(MODEL_SITES_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);

	GSList* pSites = darxen_radar_sites_get_site_list();
	while (pSites)
	{
		DarxenRadarSiteInfo* site = (DarxenRadarSiteInfo*)pSites->data;
		gtk_list_store_append(mlstSites, &iter);

		gtk_list_store_set(mlstSites, &iter,
				MODEL_SITES_ID, site->chrID,
				MODEL_SITES_STATE, site->chrState,
				MODEL_SITES_CITY, site->chrCity,
				MODEL_SITES_CURRENT, FALSE,
				-1);

		pSites = pSites->next;
	}
	mlstAvailableSites = gtk_tree_model_filter_new(GTK_TREE_MODEL(mlstSites), NULL);
	mlstCurrentSites = gtk_tree_model_filter_new(GTK_TREE_MODEL(mlstSites), NULL);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(mlstAvailableSites), is_not_current, NULL, NULL);
	gtk_tree_model_filter_set_visible_column(GTK_TREE_MODEL_FILTER(mlstCurrentSites), MODEL_SITES_CURRENT);

	g_signal_connect(G_OBJECT(mlstCurrentSites), "row-inserted", (GCallback)on_current_sites_row_inserted, NULL);
	g_signal_connect(G_OBJECT(mlstCurrentSites), "row-deleted", (GCallback)on_current_sites_row_deleted, NULL);
	g_signal_connect(G_OBJECT(mlstCurrentSites), "rows-reordered", (GCallback)on_current_sites_rows_reordered, NULL);

	lstSites = objSettingsMain->lstSites;
	while (lstSites)
	{
		SettingsSite* site = (SettingsSite*)lstSites->data;

		gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(mlstAvailableSites), &iter);
		while (valid)
		{
			gchar* id;
			gtk_tree_model_get(GTK_TREE_MODEL(mlstAvailableSites), &iter,
					MODEL_SITES_ID, &id,
					-1);

			if (!strcasecmp(id, site->name))
			{
				GtkTreeIter parentIter;
				gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(mlstAvailableSites), &parentIter, &iter);
				gtk_list_store_set(mlstSites, &parentIter,
						MODEL_SITES_CURRENT, TRUE,
						-1);
				g_free(id);
				break;
			}
			g_free(id);

			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(mlstAvailableSites), &iter);
		}
		if (!valid)
			g_warning("Site id not found: %s", site->name);

		lstSites = lstSites->next;
	}
	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(mlstAvailableSites));
	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(mlstCurrentSites));



	/* trvwAvailableSites */
	/*lstAvailableSites = objSettingsMain->lstAvailableRadarSites;
	list_store =  gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	while (lstAvailableSites)
	{
		info = (DarxenRadarSiteInfo*)lstAvailableSites->data;
		gtk_list_store_append(list_store, &iter);
		gtk_list_store_set(list_store, &iter,
						COLUMN_SITE_ID, info->chrID,
						COLUMN_STATE, info->chrState,
						COLUMN_CITY, info->chrCity,
						-1);
		lstAvailableSites = lstAvailableSites->next;
	}*/

	gtk_tree_view_set_model(GTK_TREE_VIEW(trvwAvailableSites), GTK_TREE_MODEL(mlstAvailableSites));
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(trvwAvailableSites), COLUMN_SITE_ID);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Site ID", renderer, "text", COLUMN_SITE_ID, NULL);
	gtk_tree_view_column_set_sort_column_id(column, COLUMN_SITE_ID);
	gtk_tree_view_append_column(GTK_TREE_VIEW(trvwAvailableSites), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("State", renderer, "text", COLUMN_STATE, NULL);
	gtk_tree_view_column_set_sort_column_id(column, COLUMN_STATE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(trvwAvailableSites), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("City", renderer, "text", COLUMN_CITY, NULL);
	gtk_tree_view_column_set_sort_column_id(column, COLUMN_CITY);
	gtk_tree_view_append_column(GTK_TREE_VIEW(trvwAvailableSites), column);

	gtk_drag_source_set(trvwAvailableSites, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK, siteTargets, 1, GDK_ACTION_MOVE);
	gtk_drag_dest_set(trvwAvailableSites, GTK_DEST_DEFAULT_ALL, siteTargets, 1, GDK_ACTION_MOVE);


	/* trvwCurrentSites */
	/*list_store =  gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	lstSites = settings_get_main()->lstSites;
	while (lstSites)
	{
		info = darxen_radar_sites_get_site_info(((SettingsSite*)lstSites->data)->name);
		gtk_list_store_append(list_store, &iter);
		gtk_list_store_set(list_store, &iter,
						COLUMN_SITE_ID, info->chrID,
						COLUMN_STATE, info->chrState,
						COLUMN_CITY, info->chrCity,
						-1);
		lstSites = g_list_next(lstSites);
	}*/
	gtk_tree_view_set_model(GTK_TREE_VIEW(trvwCurrentSites), GTK_TREE_MODEL(mlstCurrentSites));

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(" Site ID ", renderer, "text", COLUMN_SITE_ID, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(trvwCurrentSites), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(" State ", renderer, "text", COLUMN_STATE, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(trvwCurrentSites), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(" City ", renderer, "text", COLUMN_CITY, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(trvwCurrentSites), column);

	gtk_drag_source_set(trvwCurrentSites, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK, siteTargets, 1, GDK_ACTION_MOVE);
	gtk_drag_dest_set(trvwCurrentSites, GTK_DEST_DEFAULT_ALL, siteTargets, 1, GDK_ACTION_MOVE);

}

/**********
 * Events *
 **********/

gint
on_frmMain_delete_event(GtkWidget *widget)
{
	/*
	gint response;
	GtkWidget *dlgQuit = gtk_message_dialog_new(GTK_WINDOW(widget), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Are you sure you want to quit?");
	gtk_window_set_title(GTK_WINDOW(dlgQuit), "Really?");
	response = gtk_dialog_run(GTK_DIALOG(dlgQuit));
	gtk_widget_destroy(dlgQuit);
	if (response == GTK_RESPONSE_NO || response == GTK_RESPONSE_DELETE_EVENT)
		return TRUE;
	else
		return FALSE;
	*/
	return FALSE;
}

void
on_frmMain_destroy(GObject *widget)
{
	gtk_main_quit();
}

void
on_btnFrameFirst_clicked(GtkWidget *widget)
{
	darxen_radar_viewer_set_frame_first(DARXEN_RADAR_VIEWER(radviewCurrent));
}

void
on_btnFramePrevious_clicked(GtkWidget *widget)
{
	darxen_radar_viewer_set_frame_previous(DARXEN_RADAR_VIEWER(radviewCurrent));
}

void
on_btnFramePlay_clicked(GtkWidget *widget)
{
	darxen_radar_viewer_toggle_frame_animate(DARXEN_RADAR_VIEWER(radviewCurrent));
	if (darxen_radar_viewer_get_frame_animate(DARXEN_RADAR_VIEWER(radviewCurrent)))
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(btnFramePlay), "gtk-media-pause");
	else
		gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(btnFramePlay), "gtk-media-play");
}

void
on_btnFrameNext_clicked(GtkWidget *widget)
{
	darxen_radar_viewer_set_frame_next(DARXEN_RADAR_VIEWER(radviewCurrent));
}

void
on_btnFrameLast_clicked(GtkWidget *widget)
{
	darxen_radar_viewer_set_frame_last(DARXEN_RADAR_VIEWER(radviewCurrent));
}

void
on_mnuQuit_activate(GtkWidget *widget)
{
	gtk_widget_destroy(frmMain);
}

void
on_btnCurrentSiteMoveUp_clicked(GtkWidget *widget)
{
	GtkTreeSelection* selection;
	GtkTreeModel* model;
	GtkTreeIter selected;
	GtkTreeIter next;
	GtkTreePath* pathNext;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(trvwCurrentSites));
	if (!gtk_tree_selection_get_selected(selection, &model, &selected))
		return;
	next = selected;
	pathNext = gtk_tree_model_get_path(model, &selected);
	if (!gtk_tree_path_prev(pathNext))
	{
		gtk_tree_path_free(pathNext);
		return;
	}
	gtk_tree_model_get_iter(model, &next, pathNext);
	gtk_tree_path_free(pathNext);
	gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(model), &selected, &selected);
	gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(model), &next, &next);
	gtk_list_store_swap(GTK_LIST_STORE(gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(model))), &selected, &next);
}

void
on_btnCurrentSiteMoveDown_clicked(GtkWidget *widget)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter selected;
	GtkTreeIter next;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(trvwCurrentSites));
	if (!gtk_tree_selection_get_selected(selection, &model, &selected))
		return;
	next = selected;
	if (!gtk_tree_model_iter_next(model, &next))
		return;
	gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(model), &selected, &selected);
	gtk_tree_model_filter_convert_iter_to_child_iter(GTK_TREE_MODEL_FILTER(model), &next, &next);
	gtk_list_store_swap(GTK_LIST_STORE(gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(model))), &selected, &next);
}

void
on_btnSiteAdd_clicked(GtkWidget *widget)
{
}

void
on_btnSiteRemove_clicked(GtkWidget *widget)
{
}

void
on_mnuAbout_activate(GtkWidget *widget)
{
	GtkWidget *frmAbout;
	gint response;

	GtkBuilder* builder;
	GError* error = NULL;

	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "darxen.glade", &error);

	if (error)
	{
		g_error("Failed to load gui: %s", error->message);
	}

	gtk_builder_connect_signals(builder, NULL);

	frmAbout = (GtkWidget*)gtk_builder_get_object(builder, "frmAbout");
	gtk_widget_set_size_request(frmAbout, 300, 350);

	response = gtk_dialog_run(GTK_DIALOG(frmAbout));
	g_object_unref(builder);

	if (response == GTK_RESPONSE_CLOSE)
		gtk_widget_destroy(frmAbout);
}

void
on_current_sites_row_inserted(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	printf("Row inserted\n");fflush(stdout);

	gchar* id;
	gtk_tree_model_get(tree_model, iter,
			MODEL_SITES_ID, &id,
			-1);

	SettingsMain* settings = settings_get_main();
	GList* lstSites = settings->lstSites;
	while (lstSites)
	{
		SettingsSite* site = (SettingsSite*)lstSites->data;
		if (!strcasecmp(id, site->name))
		{
			GtkWidget* ntbkProducts;
			ntbkProducts = darxen_notebook_products_new((SettingsSite*)lstSites->data);
			g_signal_connect(DARXEN_NOTEBOOK_PRODUCTS(ntbkProducts), "page_changed", G_CALLBACK(on_ntbkProducts_page_changed), NULL);
			g_signal_connect(DARXEN_NOTEBOOK_PRODUCTS(ntbkProducts), "radar_viewer_frame_changed", G_CALLBACK(on_radar_viewer_frame_changed), NULL);
			gtk_notebook_append_page(GTK_NOTEBOOK(ntbkSites), ntbkProducts, gtk_label_new(((SettingsSite*)lstSites->data)->name));
			gtk_widget_show(ntbkProducts);
			return;
		}
		lstSites = lstSites->next;
	}

	//TODO: implement
	g_warning("Site creation not implemented");
}

void
on_current_sites_row_deleted(GtkTreeModel *tree_model, GtkTreePath *path, gpointer user_data)
{
	printf("Row deleted\n");fflush(stdout);
}

void
on_current_sites_rows_reordered(GtkTreeModel *tree_model, GtkTreePath *path, GtkTreeIter *iter, gpointer new_order, gpointer user_data)
{
	printf("Rows reordered\n");fflush(stdout);
}

void
on_tree_view_sites_drag_data_get(GtkWidget *widget, GdkDragContext *dc,
									GtkSelectionData *selection_data,
									guint info, guint t, gpointer data)
{
	GtkTreePath* path;
	//selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	//gtk_tree_drag_source_drag_data_get(GTK_TREE_DRAG_SOURCE(widget), path, selection_data);
	//gtk_tree_path_free(path);
	GtkTreeSelection *selection;
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		char *chrSite;
		char *chrState;
		char *chrCity;
		char *chrData;

		gtk_tree_model_get(	model, &iter,
							COLUMN_SITE_ID, &chrSite,
							COLUMN_STATE, &chrState,
							COLUMN_CITY, &chrCity,
							-1);

		chrData = g_strdup_printf("%s,%s,%s", chrSite, chrState, chrCity);
		gtk_selection_data_set(selection_data, selection_data->type, 0, chrData, (gint)strlen(chrData));
	}
	else
	{
		g_critical("no row selected");
	}

}

void
on_tree_view_sites_drag_data_received(GtkWidget *widget, GdkDragContext *dc,
									   gint x, gint y, GtkSelectionData *selection_data,
									   guint info, guint t, gpointer data)
{
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;

	char *chrSite;
	char *chrState;
	char *chrCity;

	chrSite = strtok(selection_data->data, ",");
	chrState = strtok(NULL, ",");
	chrCity = strtok(NULL, "\n");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(	GTK_LIST_STORE(model), &iter,
						COLUMN_SITE_ID, chrSite,
						COLUMN_STATE, chrState,
						COLUMN_CITY, chrCity,
						-1);

	if (widget == trvwCurrentSites)
	{
		/* add to the notebook */
		printf("Current sites received data\n");
	}

	g_free(selection_data->data);
}

void
on_tree_view_sites_drag_data_delete(GtkWidget *widget, GdkDragContext *dc, gpointer data)
{
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	}
	else
	{
		printf("Error, no row selected\n");
	}

	if (widget == trvwCurrentSites)
	{
		/* delete from the notebook */
		gtk_notebook_remove_page(GTK_NOTEBOOK(ntbkSites), 1);
		printf("Current sites received data\n");
	}

}

void
on_radar_viewer_frame_changed(DarxenNotebookProducts *notebook_products, DarxenRadarViewer *radview)
{
	if ((GtkWidget*)radview == radviewCurrent)
		update_frame_sensitivity();
}

void
on_ntbkProducts_page_changed(DarxenNotebookProducts *ntbk, DarxenRadarSiteInfo *site, DarxenRadarViewer *radview)
{
	radviewCurrent = (GtkWidget*)radview;
	update_frame_sensitivity();
}


/*********************
 * Private Functions *
 *********************/

void
update_frame_sensitivity()
{
	if (radviewCurrent == NULL)
	{
		gtk_widget_set_sensitive(btnFrameFirst, FALSE);
		gtk_widget_set_sensitive(btnFramePrevious, FALSE);
		gtk_widget_set_sensitive(btnFrameNext, FALSE);
		gtk_widget_set_sensitive(btnFrameLast, FALSE);
		gtk_widget_set_sensitive(btnFramePlay, FALSE);
	}
	else
	{
		int intCurrentFrame;
		int intFrameCount;

		intCurrentFrame = darxen_radar_viewer_get_frame_current(DARXEN_RADAR_VIEWER(radviewCurrent));
		intFrameCount = darxen_radar_viewer_get_frame_count(DARXEN_RADAR_VIEWER(radviewCurrent));

		gtk_widget_set_sensitive(btnFramePlay, TRUE);

		if (intCurrentFrame == 1)
		{
			gtk_widget_set_sensitive(btnFrameFirst, FALSE);
			gtk_widget_set_sensitive(btnFramePrevious, FALSE);
			gtk_widget_set_sensitive(btnFrameNext, TRUE);
			gtk_widget_set_sensitive(btnFrameLast, TRUE);
		}
		else if (intCurrentFrame == intFrameCount)
		{
			gtk_widget_set_sensitive(btnFrameFirst, TRUE);
			gtk_widget_set_sensitive(btnFramePrevious, TRUE);
			gtk_widget_set_sensitive(btnFrameNext, FALSE);
			gtk_widget_set_sensitive(btnFrameLast, FALSE);
		}
		else
		{
			gtk_widget_set_sensitive(btnFrameFirst, TRUE);
			gtk_widget_set_sensitive(btnFramePrevious, TRUE);
			gtk_widget_set_sensitive(btnFrameNext, TRUE);
			gtk_widget_set_sensitive(btnFrameLast, TRUE);
		}
	}
}


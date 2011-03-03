/* ntbkProducts.c

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

#define G_LOG_DOMAIN "DarxenNotebookProducts"
#include "glib/gmessages.h"

#include "ntbkProducts.h"

#include "libdarxenRadarSites.h"
#include "libdarxenFileSupport.h"
#include "libdarxenClient.h"

#include <gtk/gtkmarshal.h>

enum
{
	PAGE_CHANGED,
	RADAR_VIEWER_FRAME_CHANGED,
	LAST_SIGNAL
};

struct _DarxenNotebookProductsPrivate
{
	/*< private >*/
	GtkWidget *trvwProducts;
	SettingsSite site;
};

/* Events */
//static void		on_btnSiteTest_clicked	(GtkWidget *widget, gpointer user_data);

static void	on_ntbkProducts_switch_page	(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num);
static void on_radar_viewer_frame_changed(DarxenRadarViewer *radview, gpointer user_data);


static void	darxen_notebook_products_page_changed(DarxenNotebookProducts *notebook_products, DarxenRadarSiteInfo *site, DarxenRadarViewer *radview);

/* Variables */
static guint notebook_products_signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE(DarxenNotebookProducts, darxen_notebook_products, GTK_TYPE_NOTEBOOK)

static void
darxen_notebook_products_class_init(DarxenNotebookProductsClass *class)
{
	GObjectClass *gobject_class;
	GtkWidgetClass *widget_class;
	GtkNotebookClass *notebook_class;

	gobject_class = G_OBJECT_CLASS(class);
	widget_class = (GtkWidgetClass*)class;
	notebook_class = (GtkNotebookClass*)class;

	class->page_changed					= darxen_notebook_products_page_changed;
	class->radar_viewer_frame_changed	= NULL;

	notebook_products_signals[PAGE_CHANGED] = g_signal_new(
		"page_changed",
		G_TYPE_FROM_CLASS(gobject_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(DarxenNotebookProductsClass, page_changed),
		NULL, NULL,
		gtk_marshal_VOID__POINTER_POINTER,
		G_TYPE_NONE, 2,
		G_TYPE_POINTER,
		G_TYPE_POINTER);

	notebook_products_signals[RADAR_VIEWER_FRAME_CHANGED] = g_signal_new(
		"radar_viewer_frame_changed",
		G_TYPE_FROM_CLASS(gobject_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(DarxenNotebookProductsClass, radar_viewer_frame_changed),
		NULL, NULL,
		gtk_marshal_VOID__POINTER,
		G_TYPE_NONE, 1,
		G_TYPE_POINTER);
}

static void
darxen_notebook_products_init(DarxenNotebookProducts *ntbkProducts)
{

	ntbkProducts->priv = (DarxenNotebookProductsPrivate*)malloc(sizeof(DarxenNotebookProductsPrivate));

	g_signal_connect(G_OBJECT(ntbkProducts), "switch_page", G_CALLBACK(on_ntbkProducts_switch_page), NULL);
}

static void
data_received(DarxenClient* client, DarxenClientDataPacket* data, gpointer user_data)
{
	DarxenNotebookProducts* self = (DarxenNotebookProducts*)user_data;

	if (strcmp(data->site, self->priv->site.name))
		return;

	printf("Data received from %s/%s\n", data->site, data->product);fflush(stdout);

	int len;
	guchar* fdata = g_base64_decode(data->data, &len);

	FILE* f = tmpfile();

	fwrite(fdata, 1, len, f);
	fseek(f, 0, SEEK_SET);

	ProductsLevel3Data* parsed;

	if (!(parsed = parser_lvl3_parse_file(f)))
	{
		g_critical("Failed to parse level 3 data");
		return;
	}
	fclose(f);

	printf("Header: %s\n", parsed->chrWmoHeader);fflush(stdout);

	GList* lstViews = self->priv->site.views;
	int i = 1;
	while (lstViews)
	{
		SettingsView* view = (SettingsView*)lstViews->data;

		gboolean hasProduct = FALSE;
		GList* lstProducts = view->products;
		while (lstProducts && ! hasProduct)
		{
			SettingsViewProduct* product = (SettingsViewProduct*)lstProducts->data;

			hasProduct = !strcmp(product->code, data->product);

			lstProducts = lstProducts->next;
		}

		if (hasProduct)
		{
			darxen_radar_viewer_append_frame(DARXEN_RADAR_VIEWER(gtk_notebook_get_nth_page(GTK_NOTEBOOK(self), i)), parsed);
		}

		lstViews = lstViews->next;
		i++;
	}
}


/********************
 * Public Functions *
 ********************/

GtkWidget*
darxen_notebook_products_new(SettingsSite *site)
{
	GList *lstViews;

	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkListStore *list_store;
	GtkTreeIter iter;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

//	GtkWidget *lblSite;
//	GtkWidget *btnSiteTest;
	GtkWidget *radview;
	DarxenNotebookProducts *ntbkProducts;

	SettingsView *view;

	ntbkProducts = DARXEN_NOTEBOOK_PRODUCTS(gtk_widget_new(DARXEN_TYPE_NOTEBOOK_PRODUCTS, NULL));
	ntbkProducts->priv->site = *site;
	gtk_notebook_set_tab_pos((GtkNotebook*)ntbkProducts, GTK_POS_LEFT);

	hbox = gtk_hbox_new(FALSE, 0);

		vbox = gtk_vbox_new(FALSE, 0);

			label = gtk_label_new("Current Views:");
			gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 0);

			ntbkProducts->priv->trvwProducts = gtk_tree_view_new();
			list_store = gtk_list_store_new(PRODUCTS_COLUMN_COUNT, G_TYPE_STRING);
			lstViews = site->views;
			while (lstViews)
			{
				view = (SettingsView*)lstViews->data;
				gtk_list_store_append(list_store, &iter);
				gtk_list_store_set(list_store, &iter,
								PRODUCTS_COLUMN_NAME, view->name,
								-1);
				lstViews = g_list_next(lstViews);
			}
			gtk_tree_view_set_model(GTK_TREE_VIEW(ntbkProducts->priv->trvwProducts), GTK_TREE_MODEL(list_store));
			gtk_tree_view_set_search_column (GTK_TREE_VIEW(ntbkProducts->priv->trvwProducts), PRODUCTS_COLUMN_NAME);
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", PRODUCTS_COLUMN_NAME, NULL);
			gtk_tree_view_column_set_sort_column_id(column, PRODUCTS_COLUMN_NAME);
			gtk_tree_view_append_column (GTK_TREE_VIEW(ntbkProducts->priv->trvwProducts), column);
			gtk_box_pack_start(GTK_BOX(vbox), ntbkProducts->priv->trvwProducts, TRUE, TRUE, 0);

		/*hbox = gtk_hbox_new(FALSE, 0);

			lblSite = gtk_label_new(site->info->chrID);
			gtk_box_pack_start(GTK_BOX(hbox), lblSite, TRUE, TRUE, 0);

			btnSiteTest = gtk_button_new();
			gtk_button_set_label(GTK_BUTTON(btnSiteTest), "Click Me");
			gtk_box_pack_start(GTK_BOX(hbox), btnSiteTest, TRUE, TRUE, 5);
			g_signal_connect(G_OBJECT(btnSiteTest), "clicked", G_CALLBACK(on_btnSiteTest_clicked), ntbkProducts);*/

		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	gtk_notebook_append_page((GtkNotebook*)ntbkProducts, vbox, gtk_label_new("Products"));

	lstViews = site->views;
	while (lstViews != NULL)
	{
		view = (SettingsView*)lstViews->data;

		radview = darxen_radar_viewer_new(view, site);
		gtk_widget_set_size_request(radview, 200, 200);
		gtk_notebook_append_page((GtkNotebook*)ntbkProducts, radview, gtk_label_new(view->name));
		gtk_widget_show(radview);
		view->radview = DARXEN_RADAR_VIEWER(radview);
		g_signal_connect(G_OBJECT(radview), "frame_changed", G_CALLBACK(on_radar_viewer_frame_changed), ntbkProducts);

		if (view->source == VIEW_SOURCE_ARCHIVES)
		{
//			SettingsViewSourceArchive params = view->sourceParams.archive;

			if (view->viewstyle == VIEWSTYLE_SINGLE)
			{
//				GDir *dir;
//				char chrDate[9];
//				gchar *chrDir;
//				const char *chrFileName;
//				gchar *chrFilePath;
////				GList *lstFileNames;
//
//				sprintf(chrDate, "%4i%02i%02i", g_date_get_year(params.dateStart), g_date_get_month(params.dateStart), g_date_get_day(params.dateStart));
//				if (!view->products)
//					g_error("No products defined for view %s from site %s", view->name, site->name);
//				chrDir = g_build_filename(darxen_file_support_get_app_path(), "Archives", "Level 3", site->name, ((SettingsViewProduct*)view->products->data)->code, chrDate, NULL);
//
//				dir = g_dir_open(chrDir, 0, NULL);
//
//				while ((chrFileName = g_dir_read_name(dir)))
//				{
//					chrFilePath = g_build_filename(chrDir, chrFileName, NULL);
//					if (!g_file_test(chrFilePath, G_FILE_TEST_IS_DIR) && strlen(chrFileName) == 5)
//					{
//						ProductsLevel3Data *data = (ProductsLevel3Data*)malloc(sizeof(ProductsLevel3Data));
//						objFile = fopen(chrFilePath, "rb");
//						if (!parser_lvl3_parse_file(objFile, data))
//							g_error("Error loading file, %s", chrFilePath);
//						darxen_radar_viewer_append_frame(DARXEN_RADAR_VIEWER(radview), data);
//					}
//					g_free(chrFilePath);
//				}
			}
			else if (view->viewstyle == VIEWSTYLE_QUADRUPLE)
			{
				g_error("QUADRUPLE not implemented");
			}
		}
		else if (view->source == VIEW_SOURCE_LIVE)
		{
			GError* error = NULL;

			DarxenClient* client = settings_get_client();

			//FIXME: implement
			if (1) //(!darxen_client_register_poller(client, site->name, ((SettingsViewProduct*)view->products->data)->code, &error))
			{
				g_critical("Failed to add poller: %s", error->message);
			}

			g_signal_connect(client, "data-received", G_CALLBACK(data_received), ntbkProducts);
		}
		else if (view->source == VIEW_SOURCE_DIRECTORY)
		{
			gchar* dirname = g_strdup(darxen_file_support_find_path(view->sourceParams.directory.path, NULL));

			GDir *dir;
			const char *chrFileName;
			dir = g_dir_open(dirname, 0, NULL);
			while ((chrFileName = g_dir_read_name(dir)))
			{
				gchar* filePath = g_build_filename(darxen_file_support_get_overridable_file_path(dirname), chrFileName, NULL);
				if (!g_file_test(filePath, G_FILE_TEST_IS_DIR))
				{
					ProductsLevel3Data* data;
					FILE* f = fopen(filePath, "rb");
					if ((data = parser_lvl3_parse_file(f))
							&& !strcasecmp(data_wmo_get_site(data->chrWmoHeader), site->name)
							&& !strcasecmp(data_wmo_get_product_code(data->chrWmoHeader), ((SettingsViewProduct*)view->products->data)->code))
					{
						darxen_radar_viewer_append_frame(DARXEN_RADAR_VIEWER(radview), data);
					}
					else
					{
						printf("Skipping file: %s\n", filePath);
					}
					fclose(f);
				}
				g_free(filePath);
			}
			g_dir_close(dir);
			g_free(dirname);
		}

		lstViews = lstViews->next;
	}

	gtk_widget_show_all(vbox);

	return GTK_WIDGET(ntbkProducts);
}

/*
// radar viewer stuff




	DARXEN_RADAR_VIEWER_GET_CLASS(radview)->frame_changed = on_radview_frame_changed;
	darxen_radar_viewer_set_frame_last(DARXEN_RADAR_VIEWER(radview));

	gtk_widget_show(radview);*/


/**********
 * Events *
 **********/

//static void
//on_btnSiteTest_clicked(GtkWidget *widget, gpointer user_data)
//{
//	printf("Test button clicked: %s\n", ((DarxenNotebookProducts*)user_data)->priv->site.name);
//}

static void
on_ntbkProducts_switch_page(GtkNotebook *notebook, GtkNotebookPage *page, guint page_num)
{
	DarxenNotebookProducts *ntbkProducts;
	ntbkProducts = DARXEN_NOTEBOOK_PRODUCTS(notebook);

	DarxenRadarSiteInfo* info = darxen_radar_sites_get_site_info(ntbkProducts->priv->site.name);

	if (page_num == 0)
		g_signal_emit(ntbkProducts, notebook_products_signals[PAGE_CHANGED], 0, info, NULL);
	else
		g_signal_emit(ntbkProducts, notebook_products_signals[PAGE_CHANGED], 0, info, ((SettingsView*)g_list_nth_data(ntbkProducts->priv->site.views, page_num - 1))->radview);
}

static void
on_radar_viewer_frame_changed(DarxenRadarViewer *radview, gpointer user_data)
{
	g_signal_emit(user_data, notebook_products_signals[RADAR_VIEWER_FRAME_CHANGED], 0, radview);
}

static void
darxen_notebook_products_page_changed(DarxenNotebookProducts *notebook_products, DarxenRadarSiteInfo *site, DarxenRadarViewer *radview)
{
}


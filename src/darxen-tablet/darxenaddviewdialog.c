/* darxenaddviewdialog.c
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

#include "darxenaddviewdialog.h"

#include "libdarxenRadarSites.h"

static GltkScreen* screen;

static GltkDialog* dialog;
static GltkWidget* txtName;
static GltkWidget* spinnerSite;


static void
dialog_result(GltkDialog* dialog, gboolean success, gpointer user_data)
{
	gltk_screen_pop_screen(screen, GLTK_SCREEN(dialog));

	if (success)
	{
		gchar* name;
		g_object_get(txtName, "text", &name, NULL);

		const gchar* site = gltk_spinner_get_selected_item(GLTK_SPINNER(spinnerSite), 0);
		g_debug("Creating new view: %s/%s", site, name);

		darxen_config_add_view(NULL, site, name, 0);

		g_free(name);
	}

	g_object_unref(txtName);
	g_object_unref(spinnerSite);
}

void
darxen_add_view_dialog_show(GltkScreen* parent)
{
	screen = parent;
	GltkWidget* table = gltk_table_new(2, 2);

	gltk_table_set_col_options(GLTK_TABLE(table), 0, CELL_ALIGN_LEFT, FALSE, 5);
	gltk_table_set_col_options(GLTK_TABLE(table), 1, CELL_ALIGN_LEFT, FALSE, 5);
	
	gltk_table_set_row_options(GLTK_TABLE(table), 0, CELL_ALIGN_LEFT, FALSE, 5);
	gltk_table_set_row_options(GLTK_TABLE(table), 1, CELL_ALIGN_LEFT, FALSE, 5);

	{
		GltkWidget* lblName = gltk_label_new("Name: ");
		gltk_label_set_font_size(GLTK_LABEL(lblName), 28);

		txtName = gltk_entry_new("New View");
		g_object_ref(txtName);

		gltk_table_insert_widget(GLTK_TABLE(table), lblName, 0, 0);
		gltk_table_insert_widget(GLTK_TABLE(table), txtName, 1, 0);
	}
	
	{
		GltkWidget* lblSite = gltk_label_new("Site: ");
		gltk_label_set_font_size(GLTK_LABEL(lblSite), 28);

		GltkSpinnerModel* model = gltk_spinner_model_new(2);	
		GList* pSites = darxen_config_get_sites(NULL);
		for (pSites = darxen_config_get_sites(NULL); pSites; pSites = pSites->next)
		{
			DarxenSiteInfo* site = (DarxenSiteInfo*)pSites->data;

			DarxenRadarSiteInfo* siteInfo = darxen_radar_sites_get_site_info(site->name);
			gchar* display = g_strdup_printf("%s, %s", siteInfo->chrCity, siteInfo->chrState);
			gltk_spinner_model_add_toplevel(model, site->name, display);
			g_free(display);
		}

		spinnerSite = gltk_spinner_new(model);
		g_object_ref(spinnerSite);
		
		gltk_table_insert_widget(GLTK_TABLE(table), lblSite, 0, 1);
		gltk_table_insert_widget(GLTK_TABLE(table), spinnerSite, 1, 1);
	}

	dialog = gltk_dialog_new(table);
	g_signal_connect(dialog, "dialog-result", G_CALLBACK(dialog_result), NULL);
	gltk_screen_push_screen(screen, GLTK_SCREEN(dialog));
	gltk_spinner_reload_base_items(GLTK_SPINNER(spinnerSite));
}


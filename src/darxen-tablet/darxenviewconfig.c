/* darxenviewconfig.c
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

#include "darxenviewconfig.h"

#include <libdarxenShapefiles.h>

G_DEFINE_TYPE(DarxenViewConfig, darxen_view_config, GLTK_TYPE_TABLE)

#define USING_PRIVATE(obj) DarxenViewConfigPrivate* priv = DARXEN_VIEW_CONFIG_GET_PRIVATE(obj)
#define DARXEN_VIEW_CONFIG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_VIEW_CONFIG, DarxenViewConfigPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _DarxenViewConfigPrivate		DarxenViewConfigPrivate;
struct _DarxenViewConfigPrivate
{
	gchar* site;
	DarxenViewInfo* viewInfo;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void darxen_view_config_dispose(GObject* gobject);
static void darxen_view_config_finalize(GObject* gobject);

static void
darxen_view_config_class_init(DarxenViewConfigClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(DarxenViewConfigPrivate));

	gobject_class->dispose = darxen_view_config_dispose;
	gobject_class->finalize = darxen_view_config_finalize;

	klass->site_changed = NULL;
}

static void
darxen_view_config_init(DarxenViewConfig* self)
{
	USING_PRIVATE(self);

	priv->site = NULL;
	priv->viewInfo = NULL;
}

static void
darxen_view_config_dispose(GObject* gobject)
{
	G_OBJECT_CLASS(darxen_view_config_parent_class)->dispose(gobject);
}

static void
darxen_view_config_finalize(GObject* gobject)
{
	DarxenViewConfig* self = DARXEN_VIEW_CONFIG(gobject);
	USING_PRIVATE(self);

	g_free(priv->site);

	G_OBJECT_CLASS(darxen_view_config_parent_class)->finalize(gobject);
}

static void
txtName_textChanged(GltkEntry* txtName, DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);

	DarxenConfig* config = darxen_config_get_instance();
	
	gboolean res = darxen_config_rename_view(config, priv->site, priv->viewInfo, gltk_entry_get_text(txtName));

	if (!res)
	{
		//Change failed
		//TODO inform user
		gltk_button_set_text(GLTK_BUTTON(txtName), priv->viewInfo->name);
	}
}

static gboolean
btnShapefile_clicked(GltkToggleButton* btn, GltkEventClick* event, DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);

	const gchar* id = GLTK_BUTTON(btn)->text;

	GSList* pShapefiles;
	for (pShapefiles = priv->viewInfo->shapefiles; pShapefiles; pShapefiles = pShapefiles->next)
	{
		DarxenShapefile* shapefile = (DarxenShapefile*)pShapefiles->data;
		if (g_strcmp0(shapefile->name, id))
			continue;

		shapefile->visible = gltk_toggle_button_is_toggled(btn);

		return TRUE;
	}
	
	//TODO create the shapefile

	return TRUE;
}

static void
spinnerProduct_itemSelected(GltkSpinner* spinnerProduct, DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);

	DarxenConfig* config = darxen_config_get_instance();
	
	char productCode[4] = "Nxx";
	const gchar* id;
	id = gltk_spinner_get_selected_item(spinnerProduct, 0);
	productCode[2] = *id;
	id = gltk_spinner_get_selected_item(spinnerProduct, 1);
	productCode[1] = *id;
	id = gltk_spinner_get_selected_item(spinnerProduct, 2);

	g_assert(strlen(priv->viewInfo->productCode) == strlen(productCode));
	g_strlcpy(priv->viewInfo->productCode, productCode, strlen(productCode)+1);

	priv->viewInfo->smoothing = !g_strcmp0(id, "smooth");

	g_message("Product changed to %s %s", productCode, priv->viewInfo->smoothing ? "smooth" : "raw");

	darxen_config_view_updated(config, priv->site, priv->viewInfo);
}

static GList*
model_getItems(GltkSpinnerModel* model, int level, int index, GltkSpinner* spinner)
{
	GList* res = NULL;

	switch (level)
	{
		case 0:
			res = g_list_prepend(res, gltk_spinner_model_item_new("0", "0.50 Elevation"));
			res = g_list_prepend(res, gltk_spinner_model_item_new("1", "1.45 Elevation"));
			res = g_list_prepend(res, gltk_spinner_model_item_new("2", "2.40 Elevation"));
			res = g_list_prepend(res, gltk_spinner_model_item_new("3", "3.35 Elevation"));
			break;
		case 1:
			res = g_list_prepend(res, gltk_spinner_model_item_new("raw", "Raw"));
			res = g_list_prepend(res, gltk_spinner_model_item_new("smooth", "Smooth"));
			break;
		default:
			g_assert_not_reached();
	}

	return g_list_reverse(res);
}

GltkWidget*
darxen_view_config_new(gchar* site, DarxenViewInfo* viewInfo)
{
	GObject *gobject = g_object_new(DARXEN_TYPE_VIEW_CONFIG, "cols", 2, "rows", 4, NULL);
	DarxenViewConfig* self = DARXEN_VIEW_CONFIG(gobject);

	USING_PRIVATE(self);

	priv->site = g_strdup(site);
	priv->viewInfo = viewInfo;

	//Change Name - Text box
	//Change Product - Selectable/Scrollable List box OR Combo box
	//Change Source Type - Radio buttons
	//Select Date Range - Text Box (with numeric filter) OR Number spinners
	
	gltk_table_set_col_options(GLTK_TABLE(self), 0, CELL_ALIGN_LEFT, FALSE, 5);
	gltk_table_set_col_options(GLTK_TABLE(self), 1, CELL_ALIGN_LEFT, FALSE, 5);
	
	gltk_table_set_row_options(GLTK_TABLE(self), 0, CELL_ALIGN_LEFT, FALSE, 5);
	gltk_table_set_row_options(GLTK_TABLE(self), 1, CELL_ALIGN_LEFT, FALSE, 5);
	gltk_table_set_row_options(GLTK_TABLE(self), 2, CELL_ALIGN_LEFT, FALSE, 5);
	gltk_table_set_row_options(GLTK_TABLE(self), 3, CELL_ALIGN_LEFT, FALSE, 5);
	
	{
		GltkWidget* lblName = gltk_label_new("Name: ");
		gltk_label_set_font_size(GLTK_LABEL(lblName), 28);

		GltkWidget* txtName = gltk_entry_new(viewInfo->name);

		g_signal_connect(txtName, "text-changed", (GCallback)txtName_textChanged, self);

		gltk_table_insert_widget(GLTK_TABLE(self), lblName, 0, 0);
		gltk_table_insert_widget(GLTK_TABLE(self), txtName, 1, 0);
	}

	{
		GltkWidget* lblProduct = gltk_label_new("Product: ");
		gltk_label_set_font_size(GLTK_LABEL(lblProduct), 28);

		GltkSpinnerModel* model = gltk_spinner_model_new(3);	
		gltk_spinner_model_add_toplevel(model, "R", "Base Reflectivity");
		gltk_spinner_model_add_toplevel(model, "S", "Storm Velocity");

		GltkWidget* spinnerProduct = gltk_spinner_new(model);
		g_signal_connect(model, "get-items", (GCallback)model_getItems, spinnerProduct);

		char key[2] = "x";
		*key = viewInfo->productCode[2];
		gltk_spinner_set_selected_item(GLTK_SPINNER(spinnerProduct), 0, key);
		*key = viewInfo->productCode[1];
		gltk_spinner_set_selected_item(GLTK_SPINNER(spinnerProduct), 1, key);
		gltk_spinner_set_selected_item(GLTK_SPINNER(spinnerProduct), 2, viewInfo->smoothing ? "smooth" : "raw");

		g_signal_connect(spinnerProduct, "item-selected", (GCallback)spinnerProduct_itemSelected, self);
		
		gltk_table_insert_widget(GLTK_TABLE(self), lblProduct, 0, 1);
		gltk_table_insert_widget(GLTK_TABLE(self), spinnerProduct, 1, 1);
	}

	{
		GltkWidget* lblShapefiles = gltk_label_new("Shapefiles: ");
		gltk_label_set_font_size(GLTK_LABEL(lblShapefiles), 28);

		GltkWidget* shapefiles = gltk_table_new(3, 2);
		gltk_table_set_col_options(GLTK_TABLE(shapefiles), 0, CELL_ALIGN_JUSTIFY, FALSE, 5);
		gltk_table_set_col_options(GLTK_TABLE(shapefiles), 1, CELL_ALIGN_JUSTIFY, FALSE, 5);
		gltk_table_set_col_options(GLTK_TABLE(shapefiles), 2, CELL_ALIGN_JUSTIFY, FALSE, 5);
		
		gltk_table_set_row_options(GLTK_TABLE(shapefiles), 0, CELL_ALIGN_JUSTIFY, FALSE, 5);
		gltk_table_set_row_options(GLTK_TABLE(shapefiles), 1, CELL_ALIGN_JUSTIFY, FALSE, 5);

		gchar* items[] = {"Counties", "States", "Lakes", "Rivers", "Radar Sites", NULL};

		gchar** pItems;
		int x = 0;
		int y = 0;
		for (pItems = items; *pItems; pItems++)
		{
			GltkWidget* btn = gltk_toggle_button_new(*pItems);

			gboolean visible = FALSE;
			GSList* pShapefiles = priv->viewInfo->shapefiles;
			while (pShapefiles)
			{
				DarxenShapefile* shapefile = (DarxenShapefile*)pShapefiles->data;
			
				if (!g_strcmp0(shapefile->name, *pItems))
				{
					visible = shapefile->visible;
					break;
				}
			
				pShapefiles = pShapefiles->next;
			}

			gltk_toggle_button_set_toggled(GLTK_TOGGLE_BUTTON(btn), visible);
			g_signal_connect_after(btn, "click-event", (GCallback)btnShapefile_clicked, self);

			gltk_table_insert_widget(GLTK_TABLE(shapefiles), btn, x, y);
			x++;
			if (x >= 3)
			{
				x = 0;
				y++;
			}
		}

		GltkWidget* scrollable = gltk_scrollable_new();
		gltk_scrollable_set_widget(GLTK_SCROLLABLE(scrollable), shapefiles);
		
		gltk_table_insert_widget(GLTK_TABLE(self), lblShapefiles, 0, 2);
		gltk_table_insert_widget(GLTK_TABLE(self), scrollable, 1, 2);
	}

	{
		GltkWidget* lblSource = gltk_label_new("Source: ");
		gltk_label_set_font_size(GLTK_LABEL(lblSource), 28);
		
		gltk_table_insert_widget(GLTK_TABLE(self), lblSource, 0, 3);
	}

	return (GltkWidget*)gobject;
}


GQuark
darxen_view_config_error_quark()
{
	return g_quark_from_static_string("darxen-view-config-error-quark");
}

/*********************
 * Private Functions *
 *********************/


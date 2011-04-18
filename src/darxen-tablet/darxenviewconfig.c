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
#include <DarxenConversions.h>

#include <glib.h>

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
	DarxenViewInfo* origViewInfo;
	DarxenViewInfo* viewInfo;

	GltkWidget* txtName;

	GltkWidget* spinnerProduct;

	GltkWidget** btnShapefiles;

	GltkWidget* spinnerSource;

	GltkWidget* binSourceConfig;
	GltkWidget* sourceConfigArchived;

	GltkWidget* spinnerStart;
	GltkWidget* spinnerEnd;

	GltkSpinnerModel* modelStart;
	GltkSpinnerModel* modelEnd;
};

//static guint signals[LAST_SIGNAL] = {0,};

static void darxen_view_config_dispose(GObject* gobject);
static void darxen_view_config_finalize(GObject* gobject);

static void set_view_info(DarxenViewConfig* viewConfig, DarxenViewInfo* viewInfo);

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
	priv->origViewInfo = NULL;
	priv->viewInfo = NULL;

	priv->txtName = NULL;
	priv->spinnerProduct = NULL;
	priv->btnShapefiles = NULL;

	priv->spinnerSource = NULL;

	priv->binSourceConfig = NULL;
	priv->sourceConfigArchived = NULL;

	priv->spinnerStart = NULL;
	priv->spinnerEnd = NULL;
	
	priv->modelStart = NULL;
	priv->modelEnd = NULL;
}

static void
darxen_view_config_dispose(GObject* gobject)
{
	USING_PRIVATE(gobject);

	if (priv->txtName)
	{
		g_object_unref(priv->txtName);
		priv->txtName = NULL;
	}

	if (priv->spinnerProduct)
	{
		g_object_unref(priv->spinnerProduct);
		priv->spinnerProduct = NULL;
	}

	if (priv->btnShapefiles)
	{
		GltkWidget** pbtnShapefiles = priv->btnShapefiles;
		while (*pbtnShapefiles)
		{
			g_object_unref(*pbtnShapefiles);
			pbtnShapefiles++;
		}
		g_free(priv->btnShapefiles);
		priv->btnShapefiles = NULL;
	}

	if (priv->spinnerSource)
	{
		g_object_unref(priv->spinnerSource);
		priv->spinnerSource = NULL;
	}

	if (priv->binSourceConfig)
	{
		g_object_unref(priv->binSourceConfig);
		priv->binSourceConfig = NULL;
	}
	
	if (priv->sourceConfigArchived)
	{
		g_object_unref(priv->sourceConfigArchived);
		priv->sourceConfigArchived = NULL;
	}

	if (priv->spinnerStart)
	{
		g_object_unref(priv->spinnerStart);
		priv->spinnerStart = NULL;
	}

	if (priv->spinnerEnd)
	{
		g_object_unref(priv->spinnerEnd);
		priv->spinnerEnd = NULL;
	}

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

	g_free(priv->viewInfo->name);
	priv->viewInfo->name = g_strdup(gltk_entry_get_text(txtName));
}

static gboolean
btnShapefile_clicked(GltkToggleButton* btn, GltkEventClick* event, DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);

	const gchar* id = GLTK_BUTTON(btn)->text;

	//attempt to modify existing shapefile
	GSList* pShapefiles;
	for (pShapefiles = priv->viewInfo->shapefiles; pShapefiles; pShapefiles = pShapefiles->next)
	{
		DarxenShapefile* shapefile = (DarxenShapefile*)pShapefiles->data;
		if (g_strcmp0(shapefile->name, id))
			continue;

		shapefile->visible = gltk_toggle_button_is_toggled(btn);

		return TRUE;
	}
	
	//create the shapefile
	DarxenShapefile* shapefile = darxen_shapefiles_load_by_id(id);
	g_return_val_if_fail(shapefile, FALSE);
	shapefile->visible = gltk_toggle_button_is_toggled(btn);
	priv->viewInfo->shapefiles = g_slist_append(priv->viewInfo->shapefiles, shapefile);

	return TRUE;
}

static void
reload_date_range(DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);
	
	if (priv->viewInfo->sourceType != DARXEN_VIEW_SOURCE_ARCHIVE)
		return;

	DarxenConfig* config = darxen_config_get_instance();
	
	DarxenRestfulClient* client = darxen_config_get_client(config);
	int count;
	gint* years;
	years = darxen_restful_client_search_data_range(client, priv->site, priv->viewInfo->productCode,
													-1, -1, -1, &count, NULL);
	g_assert(years);
	gltk_spinner_model_clear_toplevel(priv->modelStart);
	gltk_spinner_model_clear_toplevel(priv->modelEnd);
	int i;
	for (i = 0; i < count; i++)
	{
		gchar year[5];
		sprintf(year, "%i", years[i]);
		gltk_spinner_model_add_toplevel(priv->modelStart, year, year);
		gltk_spinner_model_add_toplevel(priv->modelEnd, year, year);
	}
	g_free(years);
	gltk_spinner_reload_base_items(GLTK_SPINNER(priv->spinnerStart));
	gltk_spinner_reload_base_items(GLTK_SPINNER(priv->spinnerEnd));

	if (!g_strcmp0(priv->viewInfo->productCode, priv->origViewInfo->productCode))
	{
		DateTime startTime;
		DateTime endTime;
		g_assert(id_to_datetime(priv->origViewInfo->source.archive.startId, &startTime));
		g_assert(id_to_datetime(priv->origViewInfo->source.archive.endId, &endTime));

		char id[7];
		sprintf(id, "%i", startTime.year);
		gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerStart), 0, id);
		sprintf(id, "%i", startTime.month);
		gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerStart), 1, id);
		sprintf(id, "%i", startTime.day);
		gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerStart), 2, id);
		sprintf(id, "%i", startTime.hour * 100 + startTime.minute);
		gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerStart), 3, id);

		sprintf(id, "%i", endTime.year);
		gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerEnd), 0, id);
		sprintf(id, "%i", endTime.month);
		gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerEnd), 1, id);
		sprintf(id, "%i", endTime.day);
		gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerEnd), 2, id);
		sprintf(id, "%i", endTime.hour * 100 + endTime.minute);
		gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerEnd), 3, id);
	}
	else
	{
		for (i = 0; i < 4; i++)
		{
			gltk_spinner_set_selected_index(GLTK_SPINNER(priv->spinnerStart), i, 0);
			gltk_spinner_set_selected_index(GLTK_SPINNER(priv->spinnerEnd), i, 0);
		}
	}
}

static void
spinnerProduct_itemSelected(GltkSpinner* spinnerProduct, DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);

	char productCode[4] = "Nxx";
	const gchar* id;
	id = gltk_spinner_get_selected_item(spinnerProduct, 0);
	productCode[2] = *id;
	id = gltk_spinner_get_selected_item(spinnerProduct, 1);
	productCode[1] = *id;
	id = gltk_spinner_get_selected_item(spinnerProduct, 2);

	g_assert(strlen(priv->viewInfo->productCode) == strlen(productCode));
	gboolean reloadDates = g_strcmp0(priv->viewInfo->productCode, productCode) != 0;
	g_strlcpy(priv->viewInfo->productCode, productCode, strlen(productCode)+1);

	priv->viewInfo->smoothing = !g_strcmp0(id, "smooth");

	if (reloadDates)
		reload_date_range(viewConfig);
	//g_debug("Product changed to %s %s", productCode, priv->viewInfo->smoothing ? "smooth" : "raw");
}

static void
spinnerSource_itemSelected(GltkSpinner* spinnerSource, DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);

	const gchar* id = gltk_spinner_get_selected_item(spinnerSource, 0);

	if (!strcmp(id, "archived"))
	{
		priv->viewInfo->sourceType = DARXEN_VIEW_SOURCE_ARCHIVE;
		reload_date_range(viewConfig);
		gltk_bin_set_widget(GLTK_BIN(priv->binSourceConfig), priv->sourceConfigArchived);
	}
	else if (!strcmp(id, "live"))
	{
		priv->viewInfo->sourceType = DARXEN_VIEW_SOURCE_LIVE;
		gltk_bin_set_widget(GLTK_BIN(priv->binSourceConfig), NULL);
	}
	else
	{
		g_assert_not_reached();
	}
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

static char chrMonths[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static GList*
range_getItems(DarxenViewConfig* viewConfig, GltkSpinnerModel* model, GltkSpinner* spinner, int level, int index)
{
	USING_PRIVATE(viewConfig);

	int year = -1;
	int month = -1;
	int day = -1;

	const gchar* key;
	switch (level)
	{
		case 2:
			key = gltk_spinner_get_selected_item(spinner, 2);
			if (!key)
				return NULL;
			day = atoi(key);
		case 1:
			key = gltk_spinner_get_selected_item(spinner, 1);
			if (!key)
				return NULL;
			month = atoi(key);
		case 0:
			key = gltk_spinner_get_selected_item(spinner, 0);
			if (!key)
				return NULL;
			year = atoi(key);
			break;
		default:
			g_assert_not_reached();
	}

	DarxenRestfulClient* client = darxen_config_get_client(darxen_config_get_instance());
	int count;
	gint* ids;
	ids = darxen_restful_client_search_data_range(	client, priv->site, priv->viewInfo->productCode, 
													year, month, day, &count, NULL);
	g_assert(ids);

	GList* res = NULL;
	int i;
	for (i = 0; i < count; i++)
	{
		gchar* id = g_strdup_printf("%i", ids[i]);
		gchar* display;
		switch (level)
		{
			case 0:
				//month, convert to string
				display = g_strdup(chrMonths[ids[i]]);
				break;
			case 1:
				//day, no change
				display = g_strdup(id);
				break;
			case 2:
			{
				//time, format
				char timePart[5];
				sprintf(timePart, "%04d", ids[i]);
				display = g_strdup_printf("%.2s:%s", timePart, timePart+2);
			} break;
			default:
				g_assert_not_reached();
		}
		res = g_list_prepend(res, gltk_spinner_model_item_new(id, display));
		g_free(id);
		g_free(display);
	}
	g_free(ids);

	return g_list_reverse(res);
}

static GList*
modelStart_getItems(GltkSpinnerModel* model, int level, int index, DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);
	return range_getItems(viewConfig, model, GLTK_SPINNER(priv->spinnerStart), level, index);
}

static GList*
modelEnd_getItems(GltkSpinnerModel* model, int level, int index, DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);
	return range_getItems(viewConfig, model, GLTK_SPINNER(priv->spinnerEnd), level, index);
}

static void
spinnerStart_itemSelected(GltkSpinner* spinnerStart, DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);

	if (priv->viewInfo->source.archive.startId)
		g_free(priv->viewInfo->source.archive.startId);

	DateTime time;
	const gchar* id;
	id = gltk_spinner_get_selected_item(spinnerStart, 0);
	if (!id)
	{
		priv->viewInfo->source.archive.startId = g_strdup("");
		return;
	}
	time.year = atoi(id);
	id = gltk_spinner_get_selected_item(spinnerStart, 1);
	if (!id)
	{
		priv->viewInfo->source.archive.startId = g_strdup("");
		return;
	}
	time.month = atoi(id);
	id = gltk_spinner_get_selected_item(spinnerStart, 2);
	if (!id)
	{
		priv->viewInfo->source.archive.startId = g_strdup("");
		return;
	}
	time.day = atoi(id);
	id = gltk_spinner_get_selected_item(spinnerStart, 3);
	if (!id)
	{
		priv->viewInfo->source.archive.startId = g_strdup("");
		return;
	}
	time.hour = atoi(id);
	time.minute = time.hour % 100;
	time.hour = time.hour / 100;

	priv->viewInfo->source.archive.startId = datetime_to_id(time);
}

static void
spinnerEnd_itemSelected(GltkSpinner* spinnerEnd, DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);

	if (priv->viewInfo->source.archive.endId)
		g_free(priv->viewInfo->source.archive.endId);

	DateTime time;
	const gchar* id;
	id = gltk_spinner_get_selected_item(spinnerEnd, 0);
	if (!id)
	{
		priv->viewInfo->source.archive.endId = g_strdup("");
		return;
	}
	time.year = atoi(id);
	id = gltk_spinner_get_selected_item(spinnerEnd, 1);
	if (!id)
	{
		priv->viewInfo->source.archive.endId = g_strdup("");
		return;
	}
	time.month = atoi(id);
	id = gltk_spinner_get_selected_item(spinnerEnd, 2);
	if (!id)
	{
		priv->viewInfo->source.archive.endId = g_strdup("");
		return;
	}
	time.day = atoi(id);
	id = gltk_spinner_get_selected_item(spinnerEnd, 3);
	if (!id)
	{
		priv->viewInfo->source.archive.endId = g_strdup("");
		return;
	}
	time.hour = atoi(id);
	time.minute = time.hour % 100;
	time.hour = time.hour / 100;

	priv->viewInfo->source.archive.endId = datetime_to_id(time);
}

GltkWidget*
darxen_view_config_new(gchar* site, DarxenViewInfo* viewInfo)
{
	GObject *gobject = g_object_new(DARXEN_TYPE_VIEW_CONFIG, "cols", 2, "rows", 4, NULL);
	DarxenViewConfig* self = DARXEN_VIEW_CONFIG(gobject);

	USING_PRIVATE(self);

	priv->site = g_strdup(site);
	priv->origViewInfo = darxen_view_info_copy(viewInfo);

	//Change Name - Text box
	//Change Product - Spinner
	//Change Source Type - Toggle buttons
	//Select Date Range - Date spinners
	
	gltk_table_set_col_options(GLTK_TABLE(self), 0, CELL_ALIGN_LEFT, FALSE, 5);
	gltk_table_set_col_options(GLTK_TABLE(self), 1, CELL_ALIGN_LEFT, FALSE, 5);
	
	gltk_table_set_row_options(GLTK_TABLE(self), 0, CELL_ALIGN_LEFT, FALSE, 5);
	gltk_table_set_row_options(GLTK_TABLE(self), 1, CELL_ALIGN_LEFT, FALSE, 5);
	gltk_table_set_row_options(GLTK_TABLE(self), 2, CELL_ALIGN_LEFT, FALSE, 5);
	gltk_table_set_row_options(GLTK_TABLE(self), 3, CELL_ALIGN_LEFT, FALSE, 5);
	
	{
		GltkWidget* lblName = gltk_label_new("Name: ");
		gltk_label_set_font_size(GLTK_LABEL(lblName), 28);

		GltkWidget* txtName = gltk_entry_new("");
		g_object_ref(txtName);
		priv->txtName = txtName;

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
		g_object_ref(spinnerProduct);
		priv->spinnerProduct = spinnerProduct;
		g_signal_connect(model, "get-items", (GCallback)model_getItems, spinnerProduct);

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

		gchar* items[] = {"Counties", "States", "Lakes", "Rivers", "Sites", NULL};
		priv->btnShapefiles = g_new(GltkWidget*, 6);
		priv->btnShapefiles[5] = NULL;

		gchar** pItems;
		int i = 0;
		for (pItems = items; *pItems; pItems++)
		{
			GltkWidget* btn = gltk_toggle_button_new(*pItems);
			g_object_ref(btn);
			priv->btnShapefiles[i] = btn;

			g_signal_connect_after(btn, "click-event", (GCallback)btnShapefile_clicked, self);

			gltk_table_insert_widget(GLTK_TABLE(shapefiles), btn, i % 3, i / 3);
			i++;
		}
		
		gltk_table_insert_widget(GLTK_TABLE(self), lblShapefiles, 0, 2);
		gltk_table_insert_widget(GLTK_TABLE(self), shapefiles, 1, 2);
	}

	{

		GltkWidget* hboxSource = gltk_hbox_new(1);
		{
			priv->binSourceConfig = gltk_bin_new(NULL);
			g_object_ref(priv->binSourceConfig);
			{
				priv->sourceConfigArchived = gltk_table_new(2, 2);
				g_object_ref(priv->sourceConfigArchived);

				priv->modelStart = gltk_spinner_model_new(4);
				g_object_ref(priv->modelStart);
				priv->modelEnd = gltk_spinner_model_new(4);
				g_object_ref(priv->modelEnd);

				priv->spinnerStart = gltk_spinner_new(priv->modelStart);
				g_object_ref(priv->spinnerStart);
				priv->spinnerEnd = gltk_spinner_new(priv->modelEnd);
				g_object_ref(priv->spinnerEnd);
				
				g_signal_connect(priv->modelStart, "get-items", (GCallback)modelStart_getItems, self);
				g_signal_connect(priv->modelEnd, "get-items", (GCallback)modelEnd_getItems, self);
				g_signal_connect(priv->spinnerStart, "item-selected", (GCallback)spinnerStart_itemSelected, self);
				g_signal_connect(priv->spinnerEnd, "item-selected", (GCallback)spinnerEnd_itemSelected, self);


				gltk_table_set_col_padding(GLTK_TABLE(priv->sourceConfigArchived), 1);

				gltk_table_insert_widget(GLTK_TABLE(priv->sourceConfigArchived), gltk_label_new("Start:"), 0, 0);
				gltk_table_insert_widget(GLTK_TABLE(priv->sourceConfigArchived), gltk_label_new("End:"), 1, 0);
				gltk_table_insert_widget(GLTK_TABLE(priv->sourceConfigArchived), priv->spinnerStart, 0, 1);
				gltk_table_insert_widget(GLTK_TABLE(priv->sourceConfigArchived), priv->spinnerEnd, 1, 1);
			}

			gltk_box_append_widget(GLTK_BOX(hboxSource), priv->binSourceConfig, TRUE, TRUE);
		}
		
		GltkWidget* vboxSource = gltk_vbox_new(1);
		{
			GltkWidget* lblSource = gltk_label_new("Source: ");
			gltk_label_set_font_size(GLTK_LABEL(lblSource), 28);

			GltkSpinnerModel* model = gltk_spinner_model_new(1);
			gltk_spinner_model_add_toplevel(model, "live", "Live");
			gltk_spinner_model_add_toplevel(model, "archived", "Archived");

			GltkWidget* spinnerSource = gltk_spinner_new(model);
			g_object_ref(spinnerSource);
			priv->spinnerSource = spinnerSource;
			g_object_set(G_OBJECT(spinnerSource), "visible-items", 3, NULL);

			g_signal_connect(spinnerSource, "item-selected", (GCallback)spinnerSource_itemSelected, self);

			gltk_box_append_widget(GLTK_BOX(vboxSource), lblSource, FALSE, FALSE);
			gltk_box_append_widget(GLTK_BOX(vboxSource), spinnerSource, FALSE, FALSE);
		}
		
		gltk_table_insert_widget(GLTK_TABLE(self), vboxSource, 0, 3);
		gltk_table_insert_widget(GLTK_TABLE(self), hboxSource, 1, 3);
	}

	set_view_info(self, viewInfo);

	return (GltkWidget*)gobject;
}

void
darxen_view_config_save(DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);
	g_debug("Saving config");

	DarxenConfig* config = darxen_config_get_instance();
	if (g_strcmp0(priv->viewInfo->name, priv->origViewInfo->name))
	{
		gboolean res = darxen_config_rename_view(config, priv->site, priv->origViewInfo, priv->viewInfo->name);
		if (!res)
		{
			//invalid name (duplicate key)
			gltk_entry_set_text(GLTK_ENTRY(priv->txtName), priv->origViewInfo->name);
			g_free(priv->viewInfo->name);
			priv->viewInfo->name = g_strdup(priv->origViewInfo->name);
		}
	}

	darxen_config_view_updated(config, priv->site, priv->viewInfo->name, priv->viewInfo);

	//after
	darxen_view_info_free(priv->origViewInfo);
	priv->origViewInfo = darxen_view_info_copy(priv->viewInfo);
}

void
darxen_view_config_revert(DarxenViewConfig* viewConfig)
{
	USING_PRIVATE(viewConfig);
	g_debug("Reverting config");
	set_view_info(viewConfig, priv->origViewInfo);
}

GQuark
darxen_view_config_error_quark()
{
	return g_quark_from_static_string("darxen-view-config-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
set_view_info(DarxenViewConfig* viewConfig, DarxenViewInfo* viewInfo)
{
	USING_PRIVATE(viewConfig);

	if (priv->viewInfo)
		darxen_view_info_free(priv->viewInfo);

	priv->viewInfo = darxen_view_info_copy(viewInfo);

	//set name
	gltk_entry_set_text(GLTK_ENTRY(priv->txtName), viewInfo->name);

	//set product
	char key[2] = "x";
	*key = viewInfo->productCode[2];
	gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerProduct), 0, key);
	*key = viewInfo->productCode[1];
	gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerProduct), 1, key);
	gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerProduct), 2, viewInfo->smoothing ? "smooth" : "raw");

	//set shapefiles
	GltkWidget** pbtnShapefiles = priv->btnShapefiles;
	while (*pbtnShapefiles)
	{
		const gchar* id = GLTK_BUTTON(*pbtnShapefiles)->text;

		GSList* pShapefiles = viewInfo->shapefiles;
		DarxenShapefile* shapefile;
		while (pShapefiles)
		{
			shapefile = (DarxenShapefile*)pShapefiles->data;
		
			if (!g_strcmp0(shapefile->name, id))
				break;
		
			pShapefiles = pShapefiles->next;
		}
		if (pShapefiles)
			gltk_toggle_button_set_toggled(GLTK_TOGGLE_BUTTON(*pbtnShapefiles), shapefile->visible);
		else
			gltk_toggle_button_set_toggled(GLTK_TOGGLE_BUTTON(*pbtnShapefiles), FALSE);

		pbtnShapefiles++;
	}

	//set view source
	const char* source = viewInfo->sourceType == DARXEN_VIEW_SOURCE_LIVE ? "live" : "archived";
	gltk_spinner_set_selected_item(GLTK_SPINNER(priv->spinnerSource), 0, source);
	reload_date_range(viewConfig);

}



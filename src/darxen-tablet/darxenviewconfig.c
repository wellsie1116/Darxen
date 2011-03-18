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


G_DEFINE_TYPE(DarxenViewConfig, darxen_view_config, GLTK_TYPE_VBOX)

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

GltkWidget*
darxen_view_config_new(gchar* site, DarxenViewInfo* viewInfo)
{
	GObject *gobject = g_object_new(DARXEN_TYPE_VIEW_CONFIG, NULL);
	DarxenViewConfig* self = DARXEN_VIEW_CONFIG(gobject);

	USING_PRIVATE(self);

	priv->site = g_strdup(site);
	priv->viewInfo = viewInfo;

	//Change Name - Text box
	//Change Product - Selectable/Scrollable List box OR Combo box
	//Change Source Type - Radio buttons
	//Select Date Range - Text Box (with numeric filter) OR Number spinners
	
	GltkWidget* hboxName = gltk_hbox_new();
	{
		GltkWidget* lblName = gltk_label_new("Name: ");
		gltk_label_set_font_size(GLTK_LABEL(lblName), 28);
		GltkWidget* txtName = gltk_entry_new(viewInfo->name);

		g_signal_connect(txtName, "text-changed", (GCallback)txtName_textChanged, self);

		gltk_box_append_widget(GLTK_BOX(hboxName), lblName, FALSE, FALSE);
		gltk_box_append_widget(GLTK_BOX(hboxName), txtName, FALSE, FALSE);
	}

	GltkWidget* hboxProduct = gltk_hbox_new();
	{
		GltkWidget* lblProduct = gltk_label_new("Product: ");
		gltk_label_set_font_size(GLTK_LABEL(lblProduct), 28);
		
		gltk_box_append_widget(GLTK_BOX(hboxProduct), lblProduct, FALSE, FALSE);
	}

	GltkWidget* hboxShapefiles = gltk_hbox_new();
	{
		GltkWidget* lblShapefiles = gltk_label_new("Shapefiles: ");
		gltk_label_set_font_size(GLTK_LABEL(lblShapefiles), 28);
		
		gltk_box_append_widget(GLTK_BOX(hboxShapefiles), lblShapefiles, FALSE, FALSE);
	}

	GltkWidget* hboxSource = gltk_hbox_new();
	{
		GltkWidget* lblSource = gltk_label_new("Source: ");
		gltk_label_set_font_size(GLTK_LABEL(lblSource), 28);
		
		gltk_box_append_widget(GLTK_BOX(hboxSource), lblSource, FALSE, FALSE);
	}

	gltk_box_append_widget(GLTK_BOX(self), hboxName, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxProduct, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxShapefiles, FALSE, FALSE);
	gltk_box_append_widget(GLTK_BOX(self), hboxSource, FALSE, FALSE);

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


/* ntbkProducts.h

   Copyright (C) 2008 Kevin Wells <kevin@darxen.org>

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

#ifndef __NTBKPRODUCTS_H__
#define __NTBKPRODUCTS_H__

#include "DarxenCommon.h"
#include "DarxenSettings.h"
#include "RadarViewer.h"
#include "DarxenParserLevel3.h"

#include "libdarxenRadarSites.h"

#include <gtk/gtk.h>
#include <glade/glade-xml.h>

G_BEGIN_DECLS

#define DARXEN_TYPE_NOTEBOOK_PRODUCTS				(darxen_notebook_products_get_type())
#define DARXEN_NOTEBOOK_PRODUCTS(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_NOTEBOOK_PRODUCTS, DarxenNotebookProducts))
#define DARXEN_NOTEBOOK_PRODUCTS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_NOTEBOOK_PRODUCTS, DarxenNotebookProductsClass))
#define DARXEN_IS_NOTEBOOK_PRODUCTS(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_NOTEBOOK_PRODUCTS))
#define DARXEN_IS_NOTEBOOK_PRODUCTS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_NOTEBOOK_PRODUCTS))
#define DARXEN_NOTEBOOK_PRODUCTS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_NOTEBOOK_PRODUCTS, DarxenNotebookProductsClass))

typedef struct _DarxenNotebookProducts			DarxenNotebookProducts;
typedef struct _DarxenNotebookProductsClass		DarxenNotebookProductsClass;
typedef struct _DarxenNotebookProductsPrivate	DarxenNotebookProductsPrivate;

enum
{
	PRODUCTS_COLUMN_NAME,
	PRODUCTS_COLUMN_COUNT
};

struct _DarxenNotebookProducts
{
	GtkNotebook ntbk;

	/*< private >*/
	DarxenNotebookProductsPrivate *priv;
};

struct _DarxenNotebookProductsClass
{
	GtkNotebookClass parent_class;

	void (*page_changed)				(DarxenNotebookProducts *notebook_products, DarxenRadarSiteInfo *site, DarxenRadarViewer *radview);
	void (*radar_viewer_frame_changed)	(DarxenNotebookProducts *notebook_products, DarxenRadarViewer *radview);

};

GType      darxen_notebook_products_get_type() G_GNUC_CONST;
GtkWidget* darxen_notebook_products_new(SettingsSite *site);

G_END_DECLS

#endif

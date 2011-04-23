/* gltktable.h
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

#ifndef GLTKTABLE_H_YAT3IVVM
#define GLTKTABLE_H_YAT3IVVM

#include "gltkwidget.h"

G_BEGIN_DECLS

#define GLTK_TYPE_TABLE				(gltk_table_get_type())
#define GLTK_TABLE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_TABLE, GltkTable))
#define GLTK_TABLE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_TABLE, GltkTableClass))
#define GLTK_IS_TABLE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_TABLE))
#define GLTK_IS_TABLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_TABLE))
#define GLTK_TABLE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_TABLE, GltkTableClass))

typedef enum   _GltkTableCellAlign	GltkTableCellAlign;
typedef struct _GltkTable			GltkTable;
typedef struct _GltkTableClass		GltkTableClass;

enum _GltkTableCellAlign
{
	CELL_ALIGN_LEFT,
	CELL_ALIGN_CENTER,
	CELL_ALIGN_RIGHT,
	CELL_ALIGN_JUSTIFY
};

struct _GltkTable
{
	GltkWidget parent;
};

struct _GltkTableClass
{
	GltkWidgetClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

GType			gltk_table_get_type			() G_GNUC_CONST;
GltkWidget*		gltk_table_new				(int cols, int rows);

void			gltk_table_set_col_options	(	GltkTable* table, int i,
												GltkTableCellAlign align, gboolean expand, int padding);
void			gltk_table_set_row_options	(	GltkTable* table, int i,
												GltkTableCellAlign align, gboolean expand, int padding);

void			gltk_table_set_col_padding	(	GltkTable* table, int padding);
void			gltk_table_set_row_padding	(	GltkTable* table, int padding);

void			gltk_table_insert_widget	(GltkTable* table, GltkWidget* widget, int x, int y);
void			gltk_table_remove_widget	(GltkTable* table, int x, int y);

G_END_DECLS

#endif


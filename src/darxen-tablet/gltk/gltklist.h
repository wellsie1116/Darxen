/* gltklist.h
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

#ifndef GLTKLIST_H_BXBTDL1W
#define GLTKLIST_H_BXBTDL1W

#include "gltkscrollable.h"

G_BEGIN_DECLS

#define GLTK_LIST_ERROR gltk_list_error_quark()

#define GLTK_TYPE_LIST				(gltk_list_get_type())
#define GLTK_LIST(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_LIST, GltkList))
#define GLTK_LIST_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_LIST, GltkListClass))
#define GLTK_IS_LIST(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_LIST))
#define GLTK_IS_LIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_LIST))
#define GLTK_LIST_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_LIST, GltkListClass))

typedef struct _GltkListItem		GltkListItem;
typedef struct _GltkListItemPrivate	GltkListItemPrivate;
typedef struct _GltkList			GltkList;
typedef struct _GltkListClass		GltkListClass;

struct _GltkListItem
{
	GltkWidget* widget;
	gpointer data;
	GltkListItemPrivate* priv;
};

struct _GltkList
{
	GltkScrollable parent;
};

struct _GltkListClass
{
	GltkScrollableClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	GLTK_LIST_ERROR_FAILED
} GltkListError;

GType			gltk_list_get_type	() G_GNUC_CONST;
GltkWidget*		gltk_list_new		();

/* Public functions here */
GltkListItem*	gltk_list_add_item		(GltkList* list, GltkWidget* widget, gpointer data);
void			gltk_list_remove_item	(GltkList* list, GltkListItem* item);

GQuark			gltk_list_error_quark	();

G_END_DECLS

#endif


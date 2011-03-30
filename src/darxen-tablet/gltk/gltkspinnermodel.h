/* gltkspinnermodel.h
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

#ifndef GLTKSPINNERMODEL_H_UM6DJOD7
#define GLTKSPINNERMODEL_H_UM6DJOD7

#include <glib-object.h>

#include "gltkspinnermodel.h"

G_BEGIN_DECLS

#define GLTK_SPINNER_MODEL_ERROR gltk_spinner_model_error_quark()

#define GLTK_TYPE_SPINNER_MODEL				(gltk_spinner_model_get_type())
#define GLTK_SPINNER_MODEL(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_SPINNER_MODEL, GltkSpinnerModel))
#define GLTK_SPINNER_MODEL_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_SPINNER_MODEL, GltkSpinnerModelClass))
#define GLTK_IS_SPINNER_MODEL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_SPINNER_MODEL))
#define GLTK_IS_SPINNER_MODEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_SPINNER_MODEL))
#define GLTK_SPINNER_MODEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_SPINNER_MODEL, GltkSpinnerModelClass))

typedef struct _GltkSpinnerModelItem		GltkSpinnerModelItem;
typedef struct _GltkSpinnerModel			GltkSpinnerModel;
typedef struct _GltkSpinnerModelClass		GltkSpinnerModelClass;

struct _GltkSpinnerModelItem
{
	gchar* id;
	gchar* text;
};

struct _GltkSpinnerModel
{
	GObject parent;

	int levels;
	GList* toplevel;
};

struct _GltkSpinnerModelClass
{
	GObjectClass parent_class;
	
	/* signals */
	GList* (*get_items)	(	GltkSpinnerModel* model,
							int level,
							int index);

	/* virtual funcs */
};

typedef enum
{
	GLTK_SPINNER_MODEL_ERROR_FAILED
} GltkSpinnerModelError;

GltkSpinnerModelItem*	gltk_spinner_model_item_new		(	const gchar* id, const gchar* text);
GltkSpinnerModelItem*	gltk_spinner_model_item_clone	(	GltkSpinnerModelItem* item);
void					gltk_spinner_model_item_free	(	GltkSpinnerModelItem* item);

GType					gltk_spinner_model_get_type		(	) G_GNUC_CONST;
GltkSpinnerModel*		gltk_spinner_model_new			(	int levels);

void					gltk_spinner_model_add_toplevel	(	GltkSpinnerModel* model,
															const gchar* id,
															const gchar* text);

GltkSpinnerModelItem*	gltk_spinner_model_get_toplevel	(	GltkSpinnerModel* model,
															int index);

GList*					gltk_spinner_model_get_items	(	GltkSpinnerModel* model,
															int level,
															int index);

GList*					gltk_spinner_model_clone_items	(	GltkSpinnerModel* model,
															GList* items);

void					gltk_spinner_model_free_items	(	GltkSpinnerModel* model,
															GList* items);

GQuark					gltk_spinner_model_error_quark	(	);

G_END_DECLS

#endif


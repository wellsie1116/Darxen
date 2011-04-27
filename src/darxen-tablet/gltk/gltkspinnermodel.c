/* gltkspinnermodel.c
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

#include "gltkspinnermodel.h"

#include "gltkmarshal.h"

G_DEFINE_TYPE(GltkSpinnerModel, gltk_spinner_model, G_TYPE_OBJECT)

#define USING_PRIVATE(obj) GltkSpinnerModelPrivate* priv = GLTK_SPINNER_MODEL_GET_PRIVATE(obj)
#define GLTK_SPINNER_MODEL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_SPINNER_MODEL, GltkSpinnerModelPrivate))

enum
{
	GET_ITEMS,

	LAST_SIGNAL
};

enum
{
	PROP_0,
	PROP_LEVELS,

	N_PROPERTIES
};

typedef struct _GltkSpinnerModelPrivate		GltkSpinnerModelPrivate;
struct _GltkSpinnerModelPrivate
{
	int dummy;
};

static guint signals[LAST_SIGNAL] = {0,};
static GParamSpec* properties[N_PROPERTIES] = {0,};

static void	gltk_spinner_model_set_property	(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec);
static void	gltk_spinner_model_get_property	(GObject* object, guint property_id, GValue* value, GParamSpec* pspec);

static void gltk_spinner_model_dispose(GObject* gobject);
static void gltk_spinner_model_finalize(GObject* gobject);

static gint find_model_item(GltkSpinnerModelItem* i1, const gchar* id);

static void
gltk_spinner_model_class_init(GltkSpinnerModelClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkSpinnerModelPrivate));

	signals[GET_ITEMS] = 
		g_signal_new(	"get-items",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkSpinnerModelClass, get_items),
						NULL, NULL,
						g_cclosure_user_marshal_POINTER__INT_INT,
						G_TYPE_POINTER, 2,
						G_TYPE_INT, G_TYPE_INT);
	
	gobject_class->set_property = gltk_spinner_model_set_property;
	gobject_class->get_property = gltk_spinner_model_get_property;
	gobject_class->dispose = gltk_spinner_model_dispose;
	gobject_class->finalize = gltk_spinner_model_finalize;

	properties[PROP_LEVELS] = 
		g_param_spec_int(	"levels", "Levels", "The number of levels this model contains",
							1, 100,
							1, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

	g_object_class_install_properties(gobject_class, N_PROPERTIES, properties);
}

static void
gltk_spinner_model_init(GltkSpinnerModel* self)
{
	self->levels = 0;
	self->toplevel = NULL;
}

static void
gltk_spinner_model_dispose(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_spinner_model_parent_class)->dispose(gobject);
}

static void
gltk_spinner_model_finalize(GObject* gobject)
{
	GltkSpinnerModel* self = GLTK_SPINNER_MODEL(gobject);

	gltk_spinner_model_free_items(self, self->toplevel);

	G_OBJECT_CLASS(gltk_spinner_model_parent_class)->finalize(gobject);
}

GltkSpinnerModel*
gltk_spinner_model_new(int levels)
{
	GObject *gobject = g_object_new(GLTK_TYPE_SPINNER_MODEL, "levels", levels, NULL);
	return (GltkSpinnerModel*)gobject;
}

GltkSpinnerModelItem*
gltk_spinner_model_item_new(const gchar* id, const gchar* text)
{
	GltkSpinnerModelItem* item = g_slice_new(GltkSpinnerModelItem);

	item->id = g_strdup(id);
	item->text = g_strdup(text);

	return item;
}

GltkSpinnerModelItem*
gltk_spinner_model_item_clone(	GltkSpinnerModelItem* item)
{
	return gltk_spinner_model_item_new(item->id, item->text);
}

void
gltk_spinner_model_item_free(GltkSpinnerModelItem* item)
{
	g_free(item->id);
	g_free(item->text);
	g_slice_free(GltkSpinnerModelItem, item);
}

void
gltk_spinner_model_clear_toplevel(	GltkSpinnerModel* model)
{
	g_return_if_fail(GLTK_IS_SPINNER_MODEL(model));
	
	gltk_spinner_model_free_items(model, model->toplevel);
	model->toplevel = NULL;
}

void
gltk_spinner_model_add_toplevel	(	GltkSpinnerModel* model,
									const gchar* id,
									const gchar* text)
{
	g_return_if_fail(GLTK_IS_SPINNER_MODEL(model));
	g_return_if_fail(!g_list_find_custom(model->toplevel, id, (GCompareFunc)find_model_item));

	model->toplevel = g_list_append(model->toplevel, gltk_spinner_model_item_new(id, text));
}

GltkSpinnerModelItem*
gltk_spinner_model_get_toplevel	(	GltkSpinnerModel* model,
									int index)
{
	g_return_val_if_fail(GLTK_IS_SPINNER_MODEL(model), NULL);

	GList* item = g_list_nth(model->toplevel, index);
	g_return_val_if_fail(item, NULL);

	return (GltkSpinnerModelItem*)item->data;
}

GList*
gltk_spinner_model_get_items	(	GltkSpinnerModel* model,
									int level,
									int index)
{
	g_return_val_if_fail(GLTK_IS_SPINNER_MODEL(model), NULL);

	GList* res = NULL;

	g_object_ref(model);
	g_signal_emit(model, signals[GET_ITEMS], 0, level, index, &res);
	g_object_unref(model);

	return res;
}

GList*
gltk_spinner_model_clone_items	(	GltkSpinnerModel* model,
									GList* items)
{
	g_return_val_if_fail(GLTK_IS_SPINNER_MODEL(model), NULL);

	GList* res = g_list_copy(items);
	GList* pRes = res;
	while (pRes)
	{
		pRes->data = gltk_spinner_model_item_clone((GltkSpinnerModelItem*)pRes->data);
		pRes = pRes->next;
	}

	return res;
}

void
gltk_spinner_model_free_items	(	GltkSpinnerModel* model,
									GList* items)
{
	g_return_if_fail(GLTK_IS_SPINNER_MODEL(model));

	GList* pItems = items;
	while (pItems)
	{
		GltkSpinnerModelItem* item = (GltkSpinnerModelItem*)pItems->data;
		gltk_spinner_model_item_free(item);

		pItems = pItems->next;
	}
	g_list_free(items);
}

GQuark
gltk_spinner_model_error_quark()
{
	return g_quark_from_static_string("gltk-spinner-model-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_spinner_model_set_property(GObject* object, guint property_id, const GValue* value, GParamSpec* pspec)
{
	GltkSpinnerModel* self = GLTK_SPINNER_MODEL(object);

	switch (property_id)
	{
		case PROP_LEVELS:
		{
			self->levels = g_value_get_int(value);
		} break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void
gltk_spinner_model_get_property(GObject* object, guint property_id, GValue* value, GParamSpec* pspec)
{
	GltkSpinnerModel* self = GLTK_SPINNER_MODEL(object);

	switch (property_id)
	{
		case PROP_LEVELS:
			g_value_set_int(value, self->levels);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static gint
find_model_item(GltkSpinnerModelItem* i1, const gchar* id)
{
	return g_strcmp0(i1->id, id);
}

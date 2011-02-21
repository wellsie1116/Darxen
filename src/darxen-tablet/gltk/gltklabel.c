/* gltklabel.c
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

#include "gltklabel.h"

G_DEFINE_TYPE(GltkLabel, gltk_label, GLTK_TYPE_WIDGET)

#define USING_PRIVATE(obj) GltkLabelPrivate* priv = GLTK_LABEL_GET_PRIVATE(obj)
#define GLTK_LABEL_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_LABEL, GltkLabelPrivate))

enum
{
	LAST_SIGNAL
};

typedef struct _GltkLabelPrivate		GltkLabelPrivate;
struct _GltkLabelPrivate
{
	gchar* text;
};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_label_dispose(GObject* gobject);
static void gltk_label_finalize(GObject* gobject);

static void
gltk_label_class_init(GltkLabelClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkLabelPrivate));
	
	gobject_class->dispose = gltk_label_dispose;
	gobject_class->finalize = gltk_label_finalize;
}

static void
gltk_label_init(GltkLabel* self)
{
	USING_PRIVATE(self);

	priv->text = NULL;
}

static void
gltk_label_dispose(GObject* gobject)
{
	GltkLabel* self = GLTK_LABEL(gobject);
	USING_PRIVATE(self);

	//free and release references

	G_OBJECT_CLASS(gltk_label_parent_class)->dispose(gobject);
}

static void
gltk_label_finalize(GObject* gobject)
{
	GltkLabel* self = GLTK_LABEL(gobject);
	USING_PRIVATE(self);

	if (priv->text)
		g_free(priv->text);

	G_OBJECT_CLASS(gltk_label_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_label_new(const gchar* text)
{
	GObject *gobject = g_object_new(GLTK_TYPE_LABEL, NULL);
	GltkLabel* self = GLTK_LABEL(gobject);

	USING_PRIVATE(self);

	if (text)
		priv->text = g_strdup(text);

	return (GltkLabel*)gobject;
}


GQuark
gltk_label_error_quark()
{
	return g_quark_from_static_string("gltk-label-error-quark");
}

/*********************
 * Private Functions *
 *********************/

void
gltk_label_set_text	(const gchar* text)
{
	if (priv->text)
		g_free(priv->text);
	priv->text = g_strdup(text);
}



/* gltkbin.h
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

#ifndef GLTKBIN_H_J3VLKLLL
#define GLTKBIN_H_J3VLKLLL

#include "gltkbox.h"

G_BEGIN_DECLS

#define GLTK_BIN_ERROR gltk_bin_error_quark()

#define GLTK_TYPE_BIN				(gltk_bin_get_type())
#define GLTK_BIN(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_BIN, GltkBin))
#define GLTK_BIN_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_BIN, GltkBinClass))
#define GLTK_IS_BIN(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_BIN))
#define GLTK_IS_BIN_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_BIN))
#define GLTK_BIN_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_BIN, GltkBinClass))

typedef struct _GltkBin			GltkBin;
typedef struct _GltkBinClass		GltkBinClass;

struct _GltkBin
{
	GltkBox parent;
};

struct _GltkBinClass
{
	GltkBoxClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	GLTK_BIN_ERROR_FAILED
} GltkBinError;

GType			gltk_bin_get_type	() G_GNUC_CONST;
GltkWidget*		gltk_bin_new		(GltkWidget* widget);

void			gltk_bin_set_widget	(GltkBin* bin, GltkWidget* widget);

GQuark			gltk_bin_error_quark	();

G_END_DECLS

#endif


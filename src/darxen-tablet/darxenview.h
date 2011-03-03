/* darxenview.h
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

#ifndef DARXENVIEW_H_LBX0BK9O
#define DARXENVIEW_H_LBX0BK9O

#include <gltk/gltk.h>

#include "darxenconfig.h"

G_BEGIN_DECLS

#define DARXEN_VIEW_ERROR darxen_view_error_quark()

#define DARXEN_TYPE_VIEW				(darxen_view_get_type())
#define DARXEN_VIEW(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_VIEW, DarxenView))
#define DARXEN_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_VIEW, DarxenViewClass))
#define DARXEN_IS_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_VIEW))
#define DARXEN_IS_VIEW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_VIEW))
#define DARXEN_VIEW_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_VIEW, DarxenViewClass))

typedef struct _DarxenView			DarxenView;
typedef struct _DarxenViewClass		DarxenViewClass;

struct _DarxenView
{
	GltkVBox parent;
};

struct _DarxenViewClass
{
	GltkVBoxClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	DARXEN_VIEW_ERROR_FAILED
} DarxenViewError;

GType			darxen_view_get_type	() G_GNUC_CONST;
GltkWidget*		darxen_view_new			(DarxenViewInfo* viewInfo);

/* Public functions here */

GQuark			darxen_view_error_quark	();

G_END_DECLS

#endif


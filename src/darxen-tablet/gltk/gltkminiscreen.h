/* gltkminiscreen.h
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

#ifndef GLTKMINISCREEN_H_50GKYWNO
#define GLTKMINISCREEN_H_50GKYWNO

#include "gltkscreen.h"

G_BEGIN_DECLS

#define GLTK_MINI_SCREEN_ERROR gltk_mini_screen_error_quark()

#define GLTK_TYPE_MINI_SCREEN				(gltk_mini_screen_get_type())
#define GLTK_MINI_SCREEN(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_MINI_SCREEN, GltkMiniScreen))
#define GLTK_MINI_SCREEN_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_MINI_SCREEN, GltkMiniScreenClass))
#define GLTK_IS_MINI_SCREEN(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_MINI_SCREEN))
#define GLTK_IS_MINI_SCREEN_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_MINI_SCREEN))
#define GLTK_MINI_SCREEN_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_MINI_SCREEN, GltkMiniScreenClass))

typedef struct _GltkMiniScreen			GltkMiniScreen;
typedef struct _GltkMiniScreenClass		GltkMiniScreenClass;

struct _GltkMiniScreen
{
	GltkScreen parent;
};

struct _GltkMiniScreenClass
{
	GltkScreenClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	GLTK_MINI_SCREEN_ERROR_FAILED
} GltkMiniScreenError;

GType			gltk_mini_screen_get_type	() G_GNUC_CONST;
GltkScreen*		gltk_mini_screen_new		();

/* Public functions here */

GQuark			gltk_mini_screen_error_quark	();

G_END_DECLS

#endif


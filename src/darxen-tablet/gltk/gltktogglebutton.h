/* gltktogglebutton.h
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

#ifndef GLTKTOGGLEBUTTON_H_3LF1D5U5
#define GLTKTOGGLEBUTTON_H_3LF1D5U5

#include "gltkbutton.h"

G_BEGIN_DECLS

#define GLTK_TOGGLE_BUTTON_ERROR gltk_toggle_button_error_quark()

#define GLTK_TYPE_TOGGLE_BUTTON				(gltk_toggle_button_get_type())
#define GLTK_TOGGLE_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_TOGGLE_BUTTON, GltkToggleButton))
#define GLTK_TOGGLE_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_TOGGLE_BUTTON, GltkToggleButtonClass))
#define GLTK_IS_TOGGLE_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_TOGGLE_BUTTON))
#define GLTK_IS_TOGGLE_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_TOGGLE_BUTTON))
#define GLTK_TOGGLE_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_TOGGLE_BUTTON, GltkToggleButtonClass))

typedef struct _GltkToggleButton			GltkToggleButton;
typedef struct _GltkToggleButtonClass		GltkToggleButtonClass;

struct _GltkToggleButton
{
	GltkButton parent;
};

struct _GltkToggleButtonClass
{
	GltkButtonClass parent_class;
	
	/* signals */
	/* virtual funcs */
};

typedef enum
{
	GLTK_TOGGLE_BUTTON_ERROR_FAILED
} GltkToggleButtonError;

GType				gltk_toggle_button_get_type		() G_GNUC_CONST;
GltkWidget*			gltk_toggle_button_new			(const char* text);

gboolean			gltk_toggle_button_is_toggled	(GltkToggleButton* button);
void				gltk_toggle_button_set_toggled	(GltkToggleButton* button, gboolean value);

GQuark				gltk_toggle_button_error_quark	();

G_END_DECLS

#endif


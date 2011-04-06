/* gltkslidebutton.h
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

#ifndef GLTKCONFIGBUTTON_H_4FGLM1HC
#define GLTKCONFIGBUTTON_H_4FGLM1HC

#include "gltkslidebutton.h"

G_BEGIN_DECLS

#define GLTK_CONFIG_BUTTON_ERROR gltk_config_button_error_quark()

#define GLTK_TYPE_CONFIG_BUTTON				(gltk_config_button_get_type())
#define GLTK_CONFIG_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_CONFIG_BUTTON, GltkConfigButton))
#define GLTK_CONFIG_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_CONFIG_BUTTON, GltkConfigButtonClass))
#define GLTK_IS_CONFIG_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_CONFIG_BUTTON))
#define GLTK_IS_CONFIG_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_CONFIG_BUTTON))
#define GLTK_CONFIG_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_CONFIG_BUTTON, GltkConfigButtonClass))

typedef struct _GltkConfigButton			GltkConfigButton;
typedef struct _GltkConfigButtonClass		GltkConfigButtonClass;

struct _GltkConfigButton
{
	GltkSlideButton parent;

	gchar* displayText;
};

struct _GltkConfigButtonClass
{
	GltkSlideButtonClass parent_class;
	
	/* signals */
	void	(*config_start)	(	GltkSlideButton* button);
	
	/* virtual funcs */
};

typedef enum
{
	GLTK_CONFIG_BUTTON_ERROR_FAILED
} GltkConfigButtonError;

GType			gltk_config_button_get_type	() G_GNUC_CONST;
GltkWidget*		gltk_config_button_new		(const gchar* text, const gchar* editText);

/* Public functions here */

GQuark			gltk_config_button_error_quark	();

G_END_DECLS

#endif


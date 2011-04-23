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

#ifndef GLTKSLIDEBUTTON_H_4FGLM1HC
#define GLTKSLIDEBUTTON_H_4FGLM1HC

#include "gltkbutton.h"

#include "gltkstructs.h"

G_BEGIN_DECLS

#define GLTK_SLIDE_BUTTON_ERROR gltk_slide_button_error_quark()

#define GLTK_TYPE_SLIDE_BUTTON				(gltk_slide_button_get_type())
#define GLTK_SLIDE_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_SLIDE_BUTTON, GltkSlideButton))
#define GLTK_SLIDE_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_SLIDE_BUTTON, GltkSlideButtonClass))
#define GLTK_IS_SLIDE_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_SLIDE_BUTTON))
#define GLTK_IS_SLIDE_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_SLIDE_BUTTON))
#define GLTK_SLIDE_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_SLIDE_BUTTON, GltkSlideButtonClass))

typedef struct _GltkSlideButton			GltkSlideButton;
typedef struct _GltkSlideButtonClass		GltkSlideButtonClass;

struct _GltkSlideButton
{
	GltkButton parent;
	
	GltkColor colorText;
	GltkColor colorTextDown;

	GltkColor colorSlideLeft;
	GltkColor colorSlideRight;

	GltkColor colorHighlight;;
};

struct _GltkSlideButtonClass
{
	GltkButtonClass parent_class;
	
	/* signals */
	void (*slide_event)			(	GltkSlideButton* button,
									gboolean dirRight);

	/* virtual funcs */
};

typedef enum
{
	GLTK_SLIDE_BUTTON_ERROR_FAILED
} GltkSlideButtonError;

GType			gltk_slide_button_get_type	() G_GNUC_CONST;
GltkWidget*		gltk_slide_button_new		(const gchar* text);

/* Public functions here */

GQuark			gltk_slide_button_error_quark	();

G_END_DECLS

#endif


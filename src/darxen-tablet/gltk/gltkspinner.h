/* gltkspinner.h
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

#ifndef GLTKSPINNER_H_5DRCPDQT
#define GLTKSPINNER_H_5DRCPDQT

#include "gltkscrollable.h"

G_BEGIN_DECLS

#define GLTK_SPINNER_ERROR gltk_spinner_error_quark()

#define GLTK_TYPE_SPINNER				(gltk_spinner_get_type())
#define GLTK_SPINNER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_SPINNER, GltkSpinner))
#define GLTK_SPINNER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_SPINNER, GltkSpinnerClass))
#define GLTK_IS_SPINNER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_SPINNER))
#define GLTK_IS_SPINNER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_SPINNER))
#define GLTK_SPINNER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_SPINNER, GltkSpinnerClass))

typedef struct _GltkSpinner			GltkSpinner;
typedef struct _GltkSpinnerClass		GltkSpinnerClass;

struct _GltkSpinner
{
	GltkScrollable parent;
};

struct _GltkSpinnerClass
{
	GltkScrollableClass parent_class;
	
	/* signals */
	void	(*item_selected)	(	GltkSpinner* spinner);

	/* virtual funcs */
};

typedef enum
{
	GLTK_SPINNER_ERROR_FAILED
} GltkSpinnerError;

GType			gltk_spinner_get_type			() G_GNUC_CONST;
GltkWidget*		gltk_spinner_new				();

void			gltk_spinner_add_item			(GltkSpinner* spinner, const gchar* id, const gchar* label);
const gchar*	gltk_spinner_get_selected_item	(GltkSpinner* spinner);
void			gltk_spinner_set_selected_item	(GltkSpinner* spinner, const gchar* id);

GQuark			gltk_spinner_error_quark		();

G_END_DECLS

#endif


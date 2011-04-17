/* gltkscreen.h
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

#ifndef GLTKSCREEN_H_QIKGEARD
#define GLTKSCREEN_H_QIKGEARD

#include "gltkwidget.h"
#include "gltkwindow.h"

G_BEGIN_DECLS

#define GLTK_SCREEN_ERROR gltk_screen_error_quark()

#define GLTK_TYPE_SCREEN				(gltk_screen_get_type())
#define GLTK_SCREEN(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_SCREEN, GltkScreen))
#define GLTK_SCREEN_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_SCREEN, GltkScreenClass))
#define GLTK_IS_SCREEN(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_SCREEN))
#define GLTK_IS_SCREEN_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_SCREEN))
#define GLTK_SCREEN_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_SCREEN, GltkScreenClass))

#ifndef GLTK_SCREEN_DEF
#define GLTK_SCREEN_DEF
typedef struct _GltkScreen			GltkScreen;
#endif
typedef struct _GltkScreenClass		GltkScreenClass;

struct _GltkScreen
{
	GltkWidget  parent;

	gboolean maximized;
};

struct _GltkScreenClass
{
	GltkWidgetClass parent_class;
	
	/* signals */
	void	(*render_overlay)	(	);

	/* virtual funcs */
	void	(*layout)			(GltkScreen* screen	);
};

typedef enum
{
	GLTK_SCREEN_ERROR_FAILED
} GltkScreenError;

GType			gltk_screen_get_type	() G_GNUC_CONST;
GltkScreen*		gltk_screen_new			();

void			gltk_screen_set_window	(GltkScreen* screen, GltkWindow* window);
void			gltk_screen_set_root	(GltkScreen* screen, GltkWidget* widget);

GltkSize		gltk_screen_get_window_size			(GltkScreen* screen);

void			gltk_screen_push_screen				(GltkScreen* screen, GltkScreen* newScreen);
void			gltk_screen_pop_screen				(GltkScreen* screen, GltkScreen* newScreen);
void			gltk_screen_close_window			(GltkScreen* screen);
void			gltk_screen_invalidate				(GltkScreen* screen);
void			gltk_screen_layout					(GltkScreen* screen);
gboolean		gltk_screen_set_widget_pressed		(GltkScreen* screen, GltkWidget* widget);
void			gltk_screen_swap_widget_pressed		(GltkScreen* screen, GltkWidget* before, GltkWidget* widget);
void			gltk_screen_set_widget_unpressed	(GltkScreen* screen, GltkWidget* widget);

GltkWidget*		gltk_screen_find_drop_target		(GltkScreen* screen, const gchar* type, GltkRectangle* bounds);

GQuark			gltk_screen_error_quark	();

G_END_DECLS

#endif


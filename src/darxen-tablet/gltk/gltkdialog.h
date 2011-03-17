/* gltkdialog.h
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

#ifndef GLTKDIALOG_H_1QHSUCKQ
#define GLTKDIALOG_H_1QHSUCKQ

#include "gltkminiscreen.h"

G_BEGIN_DECLS

#define GLTK_DIALOG_ERROR gltk_dialog_error_quark()

#define GLTK_TYPE_DIALOG				(gltk_dialog_get_type())
#define GLTK_DIALOG(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_DIALOG, GltkDialog))
#define GLTK_DIALOG_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_DIALOG, GltkDialogClass))
#define GLTK_IS_DIALOG(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_DIALOG))
#define GLTK_IS_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_DIALOG))
#define GLTK_DIALOG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_DIALOG, GltkDialogClass))

typedef struct _GltkDialog			GltkDialog;
typedef struct _GltkDialogClass		GltkDialogClass;

struct _GltkDialog
{
	GltkMiniScreen parent;
};

struct _GltkDialogClass
{
	GltkMiniScreenClass parent_class;
	
	/* signals */
	void (*dialog_result)	(	GltkDialog* dialog,
								gboolean success);

	/* virtual funcs */
};

typedef enum
{
	GLTK_DIALOG_ERROR_FAILED
} GltkDialogError;

GType			gltk_dialog_get_type	() G_GNUC_CONST;
GltkDialog*		gltk_dialog_new			(GltkWidget* content);

/* Public functions here */

GQuark			gltk_dialog_error_quark	();

G_END_DECLS

#endif


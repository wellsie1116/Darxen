/* gltkdialog.c
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

#include "gltkdialog.h"
#include "gltkbin.h"
#include "gltkhbox.h"
#include "gltkvbox.h"
#include "gltkbutton.h"
#include "gltklabel.h"

G_DEFINE_TYPE(GltkDialog, gltk_dialog, GLTK_TYPE_MINI_SCREEN)

#define USING_PRIVATE(obj) GltkDialogPrivate* priv = GLTK_DIALOG_GET_PRIVATE(obj)
#define GLTK_DIALOG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_DIALOG, GltkDialogPrivate))

enum
{
	DIALOG_RESULT,

	LAST_SIGNAL
};

typedef struct _GltkDialogPrivate		GltkDialogPrivate;
struct _GltkDialogPrivate
{
	GltkWidget* content;
};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_dialog_dispose(GObject* gobject);
static void gltk_dialog_finalize(GObject* gobject);

static gboolean btnOK_clicked(GltkWidget* button, GltkEventClick* event, GltkDialog* dialog);
static gboolean btnCancel_clicked(GltkWidget* button, GltkEventClick* event, GltkDialog* dialog);

static void
gltk_dialog_class_init(GltkDialogClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkDialogPrivate));
	
	signals[DIALOG_RESULT] = 
		g_signal_new(	"dialog-result",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkDialogClass, dialog_result),
						NULL, NULL,
						g_cclosure_marshal_VOID__BOOLEAN,
						G_TYPE_NONE, 1,
						G_TYPE_BOOLEAN);
	
	gobject_class->dispose = gltk_dialog_dispose;
	gobject_class->finalize = gltk_dialog_finalize;
}

static void
gltk_dialog_init(GltkDialog* self)
{
	USING_PRIVATE(self);

	priv->content = NULL;
}

static void
gltk_dialog_dispose(GObject* gobject)
{
	GltkDialog* self = GLTK_DIALOG(gobject);
	USING_PRIVATE(self);

	if (priv->content)
	{
		g_object_unref(priv->content);
		priv->content = NULL;
	}

	G_OBJECT_CLASS(gltk_dialog_parent_class)->dispose(gobject);
}

static void
gltk_dialog_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_dialog_parent_class)->finalize(gobject);
}

GltkDialog*
gltk_dialog_new(GltkWidget* content)
{
	GObject *gobject = g_object_new(GLTK_TYPE_DIALOG, NULL);
	GltkDialog* self = GLTK_DIALOG(gobject);

	USING_PRIVATE(self);

	priv->content = gltk_bin_new(content);
	g_object_ref(priv->content);

	GltkWidget* vboxDialog = gltk_vbox_new();
	{
		GltkWidget* hboxButtons = gltk_hbox_new();
		{
			GltkWidget* btnOK = gltk_button_new("OK");
			GltkWidget* btnCancel = gltk_button_new("Cancel");

			g_signal_connect(btnOK, "click-event", (GCallback)btnOK_clicked, self);
			g_signal_connect(btnCancel, "click-event", (GCallback)btnCancel_clicked, self);

			gltk_box_append_widget(GLTK_BOX(hboxButtons), gltk_label_new(" "), TRUE, TRUE);
			gltk_box_append_widget(GLTK_BOX(hboxButtons), btnOK, FALSE, FALSE);
			gltk_box_append_widget(GLTK_BOX(hboxButtons), btnCancel, FALSE, FALSE);
		}

		gltk_box_append_widget(GLTK_BOX(vboxDialog), priv->content, TRUE, TRUE);
		gltk_box_append_widget(GLTK_BOX(vboxDialog), hboxButtons, FALSE, FALSE);
	}

	gltk_screen_set_root(GLTK_SCREEN(self), vboxDialog);

	return (GltkDialog*)gobject;
}


GQuark
gltk_dialog_error_quark()
{
	return g_quark_from_static_string("gltk-dialog-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static gboolean
btnOK_clicked(GltkWidget* button, GltkEventClick* event, GltkDialog* dialog)
{
	g_object_ref(dialog);
	g_signal_emit(dialog, signals[DIALOG_RESULT], 0, TRUE);
	g_object_unref(dialog);
	
	return TRUE;
}

static gboolean
btnCancel_clicked(GltkWidget* button, GltkEventClick* event, GltkDialog* dialog)
{
	g_object_ref(dialog);
	g_signal_emit(dialog, signals[DIALOG_RESULT], 0, FALSE);
	g_object_unref(dialog);

	return TRUE;
}


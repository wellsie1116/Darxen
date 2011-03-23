/* darxenmainview.c
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

#include "darxenmainview.h"

static GltkWidget* root = NULL;

GltkWidget*
darxen_main_view_get_root()
{
	if (!root)
	{
		root = gltk_vbox_new(0);
		g_object_ref(G_OBJECT(root));

		gltk_box_append_widget(GLTK_BOX(root), gltk_label_new("Welcome to Darxen Tablet Edition"), TRUE, TRUE);
	}

	return root;
}


/* gltkstructs.c
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

#include "gltkstructs.h"

void
gltk_color_init(GltkColor* color, float r, float g, float b)
{
	color->r = r;
	color->g = g;
	color->b = b;
}

const float*
gltk_color_get_array(GltkColor* color)
{
	return (const float*)color;
}

GltkRectangle*
gltk_rectangle_new(int x, int y, int width, int height)
{
	GltkRectangle* res = g_slice_new(GltkRectangle);
	res->x = x;
	res->y = y;
	res->width = width;
	res->height = height;
	return res;
}

GltkRectangle*
gltk_rectangle_copy(const GltkRectangle* rect)
{
	GltkRectangle* res = g_slice_new(GltkRectangle);
	*res = *rect;
	return res;
}

void
gltk_rectangle_free(GltkRectangle* rect)
{
	g_slice_free(GltkRectangle, rect);
}

gboolean
gltk_rectangle_intersects_inner(const GltkRectangle* r1, const GltkRectangle* r2)
{
	return
		((r1->x >= r2->x && r1->x <= r2->x + r2->width)
		 || (r1->x + r1->width >= r2->x && r1->x + r1->width <= r2->x + r2->width))
	 && ((r1->y >= r2->y && r1->y <= r2->y + r2->height) 
		 || (r1->y + r1->height >= r2->y && r1->y + r1->height <= r2->y + r2->height));
}

gboolean
gltk_rectangle_intersects(const GltkRectangle* r1, const GltkRectangle* r2)
{
	gboolean res = gltk_rectangle_intersects_inner(r1, r2) || gltk_rectangle_intersects_inner(r2, r1);

	//g_debug("Rectangle (%3i %3i %3i %3i) %s\nRectangle (%3i %3i %3i %3i)",
	//		r1->x, r1->y, r1->x + r1->width, r1->y + r1->height,
	//		res ? "intersects" : "does not intersect",
	//		r2->x, r2->y, r2->x + r2->width, r2->y + r2->height);

	return res; 
}


GType
gltk_rectangle_get_type()
{
	static GType type = 0;

	if (!type)
		type = g_boxed_type_register_static(g_intern_static_string("GltkRectangle"),
				(GBoxedCopyFunc)gltk_rectangle_copy,
				(GBoxedFreeFunc)gltk_rectangle_free);

	return type;
}


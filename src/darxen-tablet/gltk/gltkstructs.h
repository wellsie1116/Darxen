/* gltkstructs.h
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

#ifndef GLTKSTRUCTS_H_WZ7RM0GE
#define GLTKSTRUCTS_H_WZ7RM0GE

typedef struct _GltkColor		GltkColor;

struct _GltkColor
{
	float r;
	float g;
	float b;
};

void			gltk_color_init			(GltkColor* color, float r, float g, float b);
const float*	gltk_color_get_array	(GltkColor* color);

#endif


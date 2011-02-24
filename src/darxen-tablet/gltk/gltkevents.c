/* gltkevents.c
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

#include "gltkevents.h"

inline void
gltk_touch_free_input_event(GltkEventTouch event)
{
	g_free(event.positions);
}

gboolean
gltk_accum_event(	GSignalInvocationHint* ihint,
					GValue* return_accu,
					const GValue* handler_return,
					gpointer data)
{
	gboolean handlerReturn = g_value_get_boolean(handler_return);
	g_value_init(return_accu, G_TYPE_BOOLEAN);
	g_value_set_boolean(return_accu, handlerReturn);
	return !handlerReturn;
}

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

GltkEvent*
gltk_event_new(GltkEventType type)
{
	GltkEvent* event = g_new(GltkEvent, 1);
	event->type = type;
	return event;
}

GltkEvent*
gltk_event_clone(GltkEvent* event)
{
	GltkEvent* newEvent = gltk_event_new(event->type);
	switch (event->type)
	{
		case GLTK_TOUCH:
			newEvent->touch = event->touch;
			newEvent->touch.positions = g_new(GltkTouchPosition, event->touch.fingers);
			*(newEvent->touch.positions) = *(event->touch.positions);
			break;
		case GLTK_LONG_TOUCH:
			newEvent->longTouch = event->longTouch;
			break;
		case GLTK_DRAG:
			newEvent->drag = event->drag;
			break;
		case GLTK_PINCH:
			newEvent->pinch = event->pinch;
			break;
		case GLTK_CLICK:
			newEvent->click = event->click;
			break;
	}
	return newEvent;
}

void
gltk_event_free(GltkEvent* event)
{
	switch (event->type)
	{
		case GLTK_TOUCH:
			g_free(event->touch.positions);
			break;
		case GLTK_LONG_TOUCH:
			break;
		case GLTK_DRAG:
			break;
		case GLTK_PINCH:
			break;
		case GLTK_CLICK:
			break;
	}
	g_free(event);
}

gboolean
gltk_accum_event(	GSignalInvocationHint* ihint,
					GValue* return_accu,
					const GValue* handler_return,
					gpointer data)
{
	gboolean handlerReturn = g_value_get_boolean(handler_return);
	g_value_set_boolean(return_accu, handlerReturn);
	return !handlerReturn;
}


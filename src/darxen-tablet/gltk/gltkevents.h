/* gltkevents.h
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

#ifndef GLTKEVENTS_H_SWZAO5AI
#define GLTKEVENTS_H_SWZAO5AI

#include <glib.h>
#include <glib-object.h>

typedef enum   _GltkEventType			GltkEventType;

typedef struct _GltkEventAny			GltkEventAny;

typedef struct _GltkTouchPosition  		GltkTouchPosition;
typedef enum   _GltkTouchEventType		GltkTouchEventType;
typedef struct _GltkEventTouch			GltkEventTouch;

typedef union  _GltkEvent				GltkEvent;

enum _GltkEventType
{
	GLTK_TOUCH
};


struct _GltkEventAny
{
	GltkEventType type;
};


enum _GltkTouchEventType
{
	TOUCH_BEGIN,
	TOUCH_MOVE,
	TOUCH_END
};

struct _GltkTouchPosition
{
	int x;
	int y;
};

struct _GltkEventTouch
{
	GltkEventType type;

	int id;
	GltkTouchEventType touchType;
	int fingers;
	GltkTouchPosition* positions;
};


union _GltkEvent
{
	GltkEventType type;
	GltkEventAny any;
	GltkEventTouch touch;
};

GltkEvent*	gltk_event_new					(	GltkEventType type);
GltkEvent*	gltk_event_clone				(	GltkEvent* event);
void		gltk_event_free					(	GltkEvent* event);

gboolean	gltk_accum_event				(	GSignalInvocationHint* ihint,
												GValue* return_accu,
												const GValue* handler_return,
												gpointer data);


#endif


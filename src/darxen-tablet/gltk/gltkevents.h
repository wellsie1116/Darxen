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

#define GLTK_TYPE_EVENT					(gltk_event_get_type())
#define GLTK_TYPE_EVENT_TYPE			(gltk_event_type_get_type())

typedef enum   _GltkEventType			GltkEventType;

typedef struct _GltkEventAny			GltkEventAny;

typedef struct _GltkTouchPosition  		GltkTouchPosition;
typedef enum   _GltkTouchEventType		GltkTouchEventType;
typedef struct _GltkEventTouch			GltkEventTouch;

typedef struct _GltkEventDrag			GltkEventDrag;

typedef struct _GltkEventMultiDrag		GltkEventMultiDrag;

typedef struct _GltkEventPinch			GltkEventPinch;

typedef struct _GltkEventRotate			GltkEventRotate;

typedef struct _GltkEventClick			GltkEventClick;

typedef struct _GltkEventSlide			GltkEventSlide;

typedef union  _GltkEvent				GltkEvent;

enum _GltkEventType
{
	GLTK_TOUCH,
	GLTK_LONG_TOUCH,
	GLTK_DRAG,
	GLTK_MULTI_DRAG,
	GLTK_PINCH,
	GLTK_ROTATE,
	GLTK_CLICK,
	GLTK_SLIDE
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

struct _GltkEventDrag
{
	GltkEventType type;

	gboolean longTouched;

	int dx;
	int dy;
};

struct _GltkEventMultiDrag
{
	GltkEventType type;
	
	int fingers;
	GltkTouchPosition* positions;

	struct {
		float x;
		float y;
	} offset;

	struct {
		float x;
		float y;
	} center;
};

struct _GltkEventPinch
{
	GltkEventType type;
	
	int fingers;
	GltkTouchPosition* positions;

	float dradius;
	float radius;
	struct {
		float x;
		float y;
	} center;
};

struct _GltkEventRotate
{
	GltkEventType type;
	
	int fingers;
	GltkTouchPosition* positions;

	float dtheta;
	struct {
		float x;
		float y;
	} center;
};

struct _GltkEventClick
{
	GltkEventType type;
};

struct _GltkEventSlide
{
	GltkEventType type;

	gboolean dirRight;
};

union _GltkEvent
{
	GltkEventType type;
	GltkEventAny any;
	GltkEventTouch touch;
	GltkEventClick longTouch;
	GltkEventDrag drag;
	GltkEventMultiDrag multidrag;
	GltkEventPinch pinch;
	GltkEventRotate rotate;
	GltkEventClick click;
	GltkEventSlide slide;
};

GType		gltk_event_get_type				()	G_GNUC_CONST;
GType		gltk_event_type_get_type		()	G_GNUC_CONST;

GltkEvent*	gltk_event_new					(	GltkEventType type);
GltkEvent*	gltk_event_copy				(	GltkEvent* event);
void		gltk_event_free					(	GltkEvent* event);

gboolean	gltk_accum_event				(	GSignalInvocationHint* ihint,
												GValue* return_accu,
												const GValue* handler_return,
												gpointer data);


#endif


/* gltkslidebutton.c
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

#include "gltkslidebutton.h"

#include "gltkfonts.h"
#include "gltkmarshal.h"
#include <GL/gl.h>

G_DEFINE_TYPE(GltkSlideButton, gltk_slide_button, GLTK_TYPE_BUTTON)

#define USING_PRIVATE(obj) GltkSlideButtonPrivate* priv = GLTK_SLIDE_BUTTON_GET_PRIVATE(obj)
#define GLTK_SLIDE_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_SLIDE_BUTTON, GltkSlideButtonPrivate))

enum
{
	SLIDE_EVENT,
	LAST_SIGNAL
};

typedef struct _GltkSlideButtonPrivate		GltkSlideButtonPrivate;
struct _GltkSlideButtonPrivate
{
	float slideOffset;
};

static guint signals[LAST_SIGNAL] = {0,};

static void gltk_slide_button_dispose(GObject* gobject);
static void gltk_slide_button_finalize(GObject* gobject);

static void		gltk_slide_button_size_request	(GltkWidget* widget, GltkSize* size);
static void		gltk_slide_button_render		(GltkWidget* widget);
static gboolean	gltk_slide_button_touch_event	(GltkWidget* widget, GltkEventTouch* touch);
static gboolean	gltk_slide_button_drag_event	(GltkWidget* widget, GltkEventDrag* event);

static void
gltk_slide_button_class_init(GltkSlideButtonClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkSlideButtonPrivate));
	
	signals[SLIDE_EVENT] = 
		g_signal_new(	"slide-event",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkSlideButtonClass, slide_event),
						gltk_accum_event, NULL,
						g_cclosure_user_marshal_BOOLEAN__POINTER,
						G_TYPE_BOOLEAN, 1,
						G_TYPE_POINTER);
	
	gobject_class->dispose = gltk_slide_button_dispose;
	gobject_class->finalize = gltk_slide_button_finalize;

	gltkwidget_class->size_request = gltk_slide_button_size_request;
	gltkwidget_class->render = gltk_slide_button_render;
	gltkwidget_class->touch_event = gltk_slide_button_touch_event;
	gltkwidget_class->drag_event = gltk_slide_button_drag_event;

	klass->slide_event = NULL;

	//TODO connec to to button's clicked event so we can handle if a slide event is raised
}

static void
gltk_slide_button_init(GltkSlideButton* self)
{
	USING_PRIVATE(self);

	priv->slideOffset = 0.0f;
}

static void
gltk_slide_button_dispose(GObject* gobject)
{
	GltkSlideButton* self = GLTK_SLIDE_BUTTON(gobject);
	USING_PRIVATE(self);

	//free and release references

	G_OBJECT_CLASS(gltk_slide_button_parent_class)->dispose(gobject);
}

static void
gltk_slide_button_finalize(GObject* gobject)
{
	GltkSlideButton* self = GLTK_SLIDE_BUTTON(gobject);
	USING_PRIVATE(self);

	//free memory

	G_OBJECT_CLASS(gltk_slide_button_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_slide_button_new(const char* text)
{
	GObject *gobject = g_object_new(GLTK_TYPE_SLIDE_BUTTON, NULL);
	GltkSlideButton* self = GLTK_SLIDE_BUTTON(gobject);
	GltkButton* button = GLTK_BUTTON(gobject);

	USING_PRIVATE(self);

	button->text = g_strdup(text);

	return (GltkWidget*)gobject;
}


GQuark
gltk_slide_button_error_quark()
{
	return g_quark_from_static_string("gltk-slide-button-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_slide_button_size_request(GltkWidget* widget, GltkSize* size)
{
	GLTK_WIDGET_CLASS(gltk_slide_button_parent_class)->size_request(widget, size);
	//size->width += 50;
}

static float colorBright[] = {0.87f, 0.87f, 0.87f};
static float colorDark[] = {0.78f, 0.78f, 0.78f};

static float colorHighlightBright[] = {1.0f, 0.6f, 0.03f};
static float colorHighlightDark[] = {1.0f, 0.65f, 0.16f};

static void
gltk_slide_button_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);
	GltkButton* button = GLTK_BUTTON(widget);

	GltkAllocation allocation = gltk_widget_get_allocation(widget);
	
	glBegin(GL_LINES);
	{
		glColor3fv(colorDark);
		glVertex2i(0, 0);
		glVertex2i(allocation.width, 0);
		glVertex2i(0, allocation.height);
		glVertex2i(allocation.width, allocation.height);
	}
	glEnd();

	if (button->isDown)
	{
		glBegin(GL_QUADS);
		{
			float offset = priv->slideOffset;

			if (offset < 0.1f && offset > -0.1f)
			{
				//left gradient
				glColor3fv(colorDark);
				glVertex2i(0, 0);
				glVertex2i(0, allocation.height);
				glColor3fv(colorHighlightBright);
				glVertex2i(allocation.width * (offset+0.1), allocation.height);
				glVertex2i(allocation.width * (offset+0.1), 0);

				//slide part
				glVertex2i(allocation.width * (offset+0.1), 0);
				glVertex2i(allocation.width * (offset+0.1), allocation.height);
				glVertex2i(allocation.width + allocation.width * (offset-0.1), allocation.height);
				glVertex2i(allocation.width + allocation.width * (offset-0.1), 0);

				//right gradient
				glVertex2i(allocation.width + allocation.width * (offset-0.1), 0);
				glVertex2i(allocation.width + allocation.width * (offset-0.1), allocation.height);
				glColor3fv(colorDark);
				glVertex2i(allocation.width, allocation.height);
				glVertex2i(allocation.width, 0);
			}
			else if (offset >= 0.1f)
			{
				//render slide part
				glColor3fv(colorDark);
				glVertex2i(0, allocation.height);
				glVertex2i(0, 0);
				glVertex2i(allocation.width * (offset-0.1), 0);
				glVertex2i(allocation.width * (offset-0.1), allocation.height);
				
				//render slide gradient
				glVertex2i(allocation.width * (offset-0.1), 0);
				glVertex2i(allocation.width * (offset-0.1), allocation.height);
				glColor3fv(colorHighlightBright);
				glVertex2i(allocation.width * (offset+0.1), allocation.height);
				glVertex2i(allocation.width * (offset+0.1), 0);

				//render selection
				glVertex2i(allocation.width, 0);
				glVertex2i(allocation.width * (offset+0.1), 0);
				glColor3fv(colorHighlightBright);
				glVertex2i(allocation.width * (offset+0.1), allocation.height);
				glVertex2i(allocation.width, allocation.height);

			}
			else // (offset <= -0.1f)
			{
				//render slide part
				glColor3fv(colorDark);
				glVertex2i(allocation.width, allocation.height);
				glVertex2i(allocation.width, 0);
				glVertex2i(allocation.width + allocation.width * (offset+0.1), 0);
				glVertex2i(allocation.width + allocation.width * (offset+0.1), allocation.height);
					
				//render slide gradient
				glVertex2i(allocation.width + allocation.width * (offset+0.1), 0);
				glVertex2i(allocation.width + allocation.width * (offset+0.1), allocation.height);
				glColor3fv(colorHighlightBright);
				glVertex2i(allocation.width + allocation.width * (offset-0.1), allocation.height);
				glVertex2i(allocation.width + allocation.width * (offset-0.1), 0);

				//render selection
				glVertex2i(allocation.width + allocation.width * (offset-0.1), 0);
				glVertex2i(0, 0);
				glColor3fv(colorHighlightBright);
				glVertex2i(0, allocation.height);
				glVertex2i(allocation.width + allocation.width * (offset-0.1), allocation.height);
			}
		}
		glEnd();
	}

	glPushMatrix();
	{
		GltkGLFont* font = gltk_fonts_cache_get_font(GLTK_FONTS_BASE, 24, TRUE);
		if (button->isDown)
			glColor3f(0.0f, 0.0f, 0.0f);
		else
			glColor3fv(colorBright);

		float bbox[6];
		ftglGetFontBBox(font->font, button->text, -1, bbox);
		float height = bbox[4] - bbox[1];
		float width = bbox[3] - bbox[0];

		float x;
	   	float y;
		x = (allocation.width - width) / 2.0;
		y = (allocation.height - height) / 2.0 + font->ascender + font->descender;

		glTranslatef(x, y, 0.1f);
		glScalef(1.0f, -1.0f, 1.0f);

		ftglRenderFont(font->font, button->text, FTGL_RENDER_ALL);
	}
	glPopMatrix();
}

static gboolean
gltk_slide_button_touch_event(GltkWidget* widget, GltkEventTouch* touch)
{
	USING_PRIVATE(widget);

	if (touch->touchType == TOUCH_BEGIN)
	{
		priv->slideOffset = 0.0f;
	} 
	else if (touch->touchType == TOUCH_END)
	{
		if (priv->slideOffset > 0.5f || priv->slideOffset < -0.5f)
		{
			GltkEvent* event = gltk_event_new(GLTK_SLIDE);
			event->slide.dirRight = priv->slideOffset > 0.0f;

			gboolean returnValue;
			g_signal_emit(widget, signals[SLIDE_EVENT], 0, event, &returnValue);

			gltk_event_free(event);
		}
	}

	return GLTK_WIDGET_CLASS(gltk_slide_button_parent_class)->touch_event(widget, touch);
}

static gboolean
gltk_slide_button_drag_event(GltkWidget* widget, GltkEventDrag* event)
{
	USING_PRIVATE(widget);

	if (event->longTouched)
		return FALSE;

	GltkAllocation allocation = gltk_widget_get_allocation(widget);

	priv->slideOffset += (float)event->dx / allocation.width;
	priv->slideOffset = CLAMP(priv->slideOffset, -0.9, 0.9);

	gltk_widget_invalidate(widget);

	//don't consume the event
	return FALSE;
}


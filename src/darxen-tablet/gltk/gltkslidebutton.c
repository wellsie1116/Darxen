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
						NULL, NULL,
						g_cclosure_user_marshal_NONE__BOOL,
						G_TYPE_NONE, 1,
						G_TYPE_BOOLEAN);
	
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

	gltk_color_init(&self->colorText, 0.87f, 0.87f, 0.87f);
	gltk_color_init(&self->colorTextDown, 0.0f, 0.0f, 0.0f);

	gltk_color_init(&self->colorSlideLeft, 0.78f, 0.78f, 0.78f);
	gltk_color_init(&self->colorSlideRight, 0.78f, 0.78f, 0.78f);
	
	gltk_color_init(&self->colorHighlight, 1.0f, 0.6f, 0.03f);

	priv->slideOffset = 0.0f;
}

static void
gltk_slide_button_dispose(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_slide_button_parent_class)->dispose(gobject);
}

static void
gltk_slide_button_finalize(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_slide_button_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_slide_button_new(const char* text)
{
	GObject *gobject = g_object_new(GLTK_TYPE_SLIDE_BUTTON, NULL);
	GltkButton* button = GLTK_BUTTON(gobject);

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

static void
gltk_slide_button_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);
	GltkButton* button = GLTK_BUTTON(widget);
	GltkSlideButton* slideButton = GLTK_SLIDE_BUTTON(widget);

	GltkAllocation allocation = gltk_widget_get_allocation(widget);
	
	glBegin(GL_LINES);
	{
		glColor3fv(gltk_color_get_array(&slideButton->colorSlideLeft));
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
				glColor3fv(gltk_color_get_array(&slideButton->colorSlideRight));
				glVertex2i(0, 0);
				glVertex2i(0, allocation.height);
				glColor3fv(gltk_color_get_array(&slideButton->colorHighlight));
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
				glColor3fv(gltk_color_get_array(&slideButton->colorSlideLeft));
				glVertex2i(allocation.width, allocation.height);
				glVertex2i(allocation.width, 0);
			}
			else if (offset >= 0.1f)
			{
				//render slide part
				glColor3fv(gltk_color_get_array(&slideButton->colorSlideRight));
				glVertex2i(0, allocation.height);
				glVertex2i(0, 0);
				glVertex2i(allocation.width * (offset-0.1), 0);
				glVertex2i(allocation.width * (offset-0.1), allocation.height);
				
				//render slide gradient
				glVertex2i(allocation.width * (offset-0.1), 0);
				glVertex2i(allocation.width * (offset-0.1), allocation.height);
				glColor3fv(gltk_color_get_array(&slideButton->colorHighlight));
				glVertex2i(allocation.width * (offset+0.1), allocation.height);
				glVertex2i(allocation.width * (offset+0.1), 0);

				//render selection
				glVertex2i(allocation.width, 0);
				glVertex2i(allocation.width * (offset+0.1), 0);
				glColor3fv(gltk_color_get_array(&slideButton->colorHighlight));
				glVertex2i(allocation.width * (offset+0.1), allocation.height);
				glVertex2i(allocation.width, allocation.height);

			}
			else // (offset <= -0.1f)
			{
				//render slide part
				glColor3fv(gltk_color_get_array(&slideButton->colorSlideLeft));
				glVertex2i(allocation.width, allocation.height);
				glVertex2i(allocation.width, 0);
				glVertex2i(allocation.width + allocation.width * (offset+0.1), 0);
				glVertex2i(allocation.width + allocation.width * (offset+0.1), allocation.height);
					
				//render slide gradient
				glVertex2i(allocation.width + allocation.width * (offset+0.1), 0);
				glVertex2i(allocation.width + allocation.width * (offset+0.1), allocation.height);
				glColor3fv(gltk_color_get_array(&slideButton->colorHighlight));
				glVertex2i(allocation.width + allocation.width * (offset-0.1), allocation.height);
				glVertex2i(allocation.width + allocation.width * (offset-0.1), 0);

				//render selection
				glVertex2i(allocation.width + allocation.width * (offset-0.1), 0);
				glVertex2i(0, 0);
				glColor3fv(gltk_color_get_array(&slideButton->colorHighlight));
				glVertex2i(0, allocation.height);
				glVertex2i(allocation.width + allocation.width * (offset-0.1), allocation.height);
			}
		}
		glEnd();

		glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
		glBegin(GL_TRIANGLES);
		{
			float offset1 = 0.05f;
			float offset2 = 0.05f;
			static const float arrowLen = 0.05f;
			if (priv->slideOffset < 0.0f)
				if (priv->slideOffset < -0.5f)
					offset1 = 0.0f;
				else
					offset1 = 0.05 - -0.1f * (priv->slideOffset);
			else
				if (priv->slideOffset > 0.5f)
					offset2 = 0.0f;
				else
					offset2 = 0.05f - 0.1f * priv->slideOffset;

			glVertex2i(allocation.width * offset1, 0.5 * allocation.height);
			glVertex2i(allocation.width * (offset1+arrowLen), 0.3 * allocation.height);
			glVertex2i(allocation.width * (offset1+arrowLen), 0.7 * allocation.height);
			
			glVertex2i(allocation.width * (1.0f-offset2), 0.5 * allocation.height);
			glVertex2i(allocation.width * (1.0f-(offset2+arrowLen)), 0.3 * allocation.height);
			glVertex2i(allocation.width * (1.0f-(offset2+arrowLen)), 0.7 * allocation.height);
		}
		glEnd();
	}


	glPushMatrix();
	{
		GltkGLFont* font = gltk_fonts_cache_get_font(GLTK_FONTS_BASE, 24, TRUE);
		if (button->isDown)
			glColor3fv(gltk_color_get_array(&slideButton->colorTextDown));
		else
			glColor3fv(gltk_color_get_array(&slideButton->colorText));

		GltkGLFontBounds bounds = gltk_fonts_measure_string(font, GLTK_BUTTON(widget)->text);
		float height = bounds.height;
		float width = bounds.width;

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
			gboolean dirRight = priv->slideOffset > 0.0f;
			g_signal_emit(widget, signals[SLIDE_EVENT], 0, dirRight);
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


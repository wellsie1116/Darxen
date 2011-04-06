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

#include "gltkconfigbutton.h"

#include "gltkfonts.h"
#include "gltkmarshal.h"
#include <GL/gl.h>

G_DEFINE_TYPE(GltkConfigButton, gltk_config_button, GLTK_TYPE_SLIDE_BUTTON)

#define USING_PRIVATE(obj) GltkConfigButtonPrivate* priv = GLTK_CONFIG_BUTTON_GET_PRIVATE(obj)
#define GLTK_CONFIG_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GLTK_TYPE_CONFIG_BUTTON, GltkConfigButtonPrivate))

enum
{
	CONFIG_START,

	LAST_SIGNAL
};

enum
{
	PROP_0,
	PROP_CONFIG_MODE,
	PROP_DISPLAY_TEXT,

	N_PROPERTIES
};

typedef struct _GltkConfigButtonPrivate		GltkConfigButtonPrivate;
struct _GltkConfigButtonPrivate
{
	guint renderOverlayId;

	gboolean isConfig;

	float slideOffset;
	GTimer* dragTimer;
	guint animDrag;
	gfloat dOffset;
};

static guint signals[LAST_SIGNAL] = {0,};
static GParamSpec* properties[N_PROPERTIES] = {0,};

static void gltk_config_button_dispose(GObject* gobject);
static void gltk_config_button_finalize(GObject* gobject);

static void	gltk_config_button_set_property	(	GObject* object, guint property_id,
												const GValue* value, GParamSpec* pspec);
static void	gltk_config_button_get_property	(	GObject* object, guint property_id,
												GValue* value, GParamSpec* pspec);

static void		gltk_config_button_size_request		(GltkWidget* widget, GltkSize* size);
static void		gltk_config_button_set_screen		(GltkWidget* widget, GltkScreen* screen);
static void		gltk_config_button_render			(GltkWidget* widget);
static void		gltk_config_button_render_overlay	(GltkScreen* screen, GltkWidget* widget);
static gboolean	gltk_config_button_touch_event		(GltkWidget* widget, GltkEventTouch* touch);
static gboolean	gltk_config_button_drag_event		(GltkWidget* widget, GltkEventDrag* event);
static gboolean	gltk_config_button_click_event		(GltkWidget* widget, GltkEventClick* event);

static void		gltk_config_button_slide_event		(GltkSlideButton* widget, gboolean dirRight);

static void
gltk_config_button_class_init(GltkConfigButtonClass* klass)
{
	GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
	GltkWidgetClass* gltkwidget_class = GLTK_WIDGET_CLASS(klass);
	GltkSlideButtonClass* gltkslidebutton_class = GLTK_SLIDE_BUTTON_CLASS(klass);

	g_type_class_add_private(klass, sizeof(GltkConfigButtonPrivate));
	
	signals[CONFIG_START] = 
		g_signal_new(	"config-start",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(GltkConfigButtonClass, config_start),
						NULL, NULL,
						g_cclosure_user_marshal_NONE__NONE,
						G_TYPE_NONE, 0);
	
	gobject_class->dispose = gltk_config_button_dispose;
	gobject_class->finalize = gltk_config_button_finalize;
	gobject_class->set_property = gltk_config_button_set_property;
	gobject_class->get_property = gltk_config_button_get_property;

	gltkwidget_class->size_request = gltk_config_button_size_request;
	gltkwidget_class->set_screen = gltk_config_button_set_screen;
	gltkwidget_class->render = gltk_config_button_render;
	gltkwidget_class->touch_event = gltk_config_button_touch_event;
	gltkwidget_class->drag_event = gltk_config_button_drag_event;
	gltkwidget_class->click_event = gltk_config_button_click_event;

	gltkslidebutton_class->slide_event = gltk_config_button_slide_event;

	klass->config_start = NULL;
	
	properties[PROP_CONFIG_MODE] = 
		g_param_spec_boolean(	"config-mode", "Configuration Mode",
								"Flag specifying whether or not button is in config mode",
								FALSE, G_PARAM_READWRITE);

	properties[PROP_DISPLAY_TEXT] = 
		g_param_spec_string(	"display-text", "Display Text", "Text to display on top of the button",
								"", G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

	g_object_class_install_properties(gobject_class, N_PROPERTIES, properties);

	//TODO connect to to button's clicked event so we can handle if a slide event is raised
}

static void
gltk_config_button_init(GltkConfigButton* self)
{
	GltkSlideButton* slideButton = GLTK_SLIDE_BUTTON(self);
	USING_PRIVATE(self);

	self->displayText = NULL;

	gltk_color_init(&slideButton->colorText, 0.87f, 0.87f, 0.87f);
	gltk_color_init(&slideButton->colorTextDown, 0.87f, 0.87f, 0.87f);

	gltk_color_init(&slideButton->colorSlideLeft, 1.0f, 0.2f, 0.2f);
	gltk_color_init(&slideButton->colorSlideRight, 0.2f, 1.0f, 0.2f);
	
	gltk_color_init(&slideButton->colorHighlight, 0.0f, 0.0f, 0.0f);

	priv->isConfig = FALSE;

	priv->slideOffset = 0.0f;
	priv->dragTimer = g_timer_new();
	priv->animDrag = 0;
	priv->dOffset = 0.0f;
}

static void
gltk_config_button_dispose(GObject* gobject)
{
	G_OBJECT_CLASS(gltk_config_button_parent_class)->dispose(gobject);
}

static void
gltk_config_button_finalize(GObject* gobject)
{
	GltkConfigButton* self = GLTK_CONFIG_BUTTON(gobject);
	USING_PRIVATE(self);

	if (self->displayText)
		g_free(self->displayText);

	g_timer_destroy(priv->dragTimer);

	G_OBJECT_CLASS(gltk_config_button_parent_class)->finalize(gobject);
}

GltkWidget*
gltk_config_button_new(const char* text, const gchar* displayText)
{
	return (GltkWidget*)g_object_new(	GLTK_TYPE_CONFIG_BUTTON,
										"text",			text, 
										"display-text",	displayText, 
										NULL);
}

GQuark
gltk_config_button_error_quark()
{
	return g_quark_from_static_string("gltk-slide-button-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
gltk_config_button_set_property	(	GObject* object, guint property_id,
									const GValue* value, GParamSpec* pspec)
{
	GltkConfigButton* self = GLTK_CONFIG_BUTTON(object);
	USING_PRIVATE(object);

	switch (property_id)
	{
		case PROP_CONFIG_MODE:
		{
			gboolean config = g_value_get_boolean(value);
			if (config && !priv->isConfig)
			{
				g_signal_emit(object, signals[CONFIG_START], 0);
				priv->animDrag = 0;
				priv->isConfig = config;
			}
			else if (!config && priv->isConfig)
			{
				priv->isConfig = config;
			}
			else
			{
				break;
			}
			gltk_widget_invalidate(GLTK_WIDGET(self));
		} break;
		case PROP_DISPLAY_TEXT:
			if (self->displayText)
				g_free(self->displayText);
			self->displayText = g_value_dup_string(value);
			gltk_widget_invalidate(GLTK_WIDGET(self));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void
gltk_config_button_get_property	(	GObject* object, guint property_id,
									GValue* value, GParamSpec* pspec)
{
	GltkConfigButton* self = GLTK_CONFIG_BUTTON(object);
	USING_PRIVATE(object);

	switch (property_id)
	{
		case PROP_CONFIG_MODE:
			g_value_set_boolean(value, priv->isConfig);
			break;
		case PROP_DISPLAY_TEXT:
			g_value_set_string(value, self->displayText);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void
gltk_config_button_size_request(GltkWidget* widget, GltkSize* size)
{
	GltkConfigButton* configButton = GLTK_CONFIG_BUTTON(widget);

	GLTK_WIDGET_CLASS(gltk_config_button_parent_class)->size_request(widget, size);

	GltkSize overlaySize;
	GltkGLFont* font = gltk_fonts_cache_get_font(GLTK_FONTS_BASE, 24, TRUE);

	GltkGLFontBounds bounds = gltk_fonts_measure_string(font, configButton->displayText);
	overlaySize.width = bounds.width;
	overlaySize.height = bounds.height;

	overlaySize.width += 50;

	size->width = MAX(size->width, overlaySize.width);
	size->height = MAX(size->height, overlaySize.height);
}

static void
gltk_config_button_set_screen(GltkWidget* widget, GltkScreen* screen)
{
	USING_PRIVATE(widget);
     
	//setup/teardown hook for render-overlay
	if (!screen && priv->renderOverlayId)
	{
		g_signal_handler_disconnect(widget->screen, priv->renderOverlayId);
		priv->renderOverlayId = 0;
	}
	else if (screen)
	{
		priv->renderOverlayId = g_signal_connect(	screen, "render-overlay",
													(GCallback)gltk_config_button_render_overlay, widget);   
	}

	GLTK_WIDGET_CLASS(gltk_config_button_parent_class)->set_screen(widget, screen);
}

static float colorBright[] = {0.87f, 0.87f, 0.87f};
static float colorDark[] = {0.78f, 0.78f, 0.78f};

//static float colorHighlightBright[] = {1.0f, 0.6f, 0.03f};
//static float colorHighlightDark[] = {1.0f, 0.65f, 0.16f};

static void
render_overlay(GltkWidget* widget)
{
	USING_PRIVATE(widget);
	GltkConfigButton* configButton = GLTK_CONFIG_BUTTON(widget);

	GltkAllocation allocation = gltk_widget_get_allocation(widget);

	//render background
	glColor3f(0.0f, 0.0f, 0.0f);
	glRectf(0, 0, allocation.width, allocation.height);
	
	//render arrow
	glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
	glBegin(GL_TRIANGLES);
	{
		float offset2 = 0.05f;
		static const float arrowLen = 0.05f;
		if (priv->slideOffset > 0.5f)
			offset2 = 0.0f;
		else
			offset2 = 0.05f - 0.1f * priv->slideOffset;

		glVertex2i(allocation.width * (1.0f-offset2), 0.5 * allocation.height);
		glVertex2i(allocation.width * (1.0f-(offset2+arrowLen)), 0.3 * allocation.height);
		glVertex2i(allocation.width * (1.0f-(offset2+arrowLen)), 0.7 * allocation.height);
	}
	glEnd();
	
	//render text
	glPushMatrix();
	{
		GltkGLFont* font = gltk_fonts_cache_get_font(GLTK_FONTS_BASE, 24, TRUE);
		glColor3fv(colorBright);

		GltkGLFontBounds bounds = gltk_fonts_measure_string(font, configButton->displayText);
		float height = bounds.height;
		float width = bounds.width;

		float x;
	   	float y;
		x = (allocation.width - width) - 50;// / 2.0;
		y = (allocation.height - height) / 2.0 + font->ascender + font->descender;

		glTranslatef(x, y, 0.1f);
		glScalef(1.0f, -1.0f, 1.0f);

		ftglRenderFont(font->font, configButton->displayText, FTGL_RENDER_ALL);
	}
	glPopMatrix();

}

static void
gltk_config_button_render(GltkWidget* widget)
{
	USING_PRIVATE(widget);
	GltkButton* button = GLTK_BUTTON(widget);

	//render the main part
	if (button->isDown || priv->animDrag || priv->isConfig)
		GLTK_WIDGET_CLASS(gltk_config_button_parent_class)->render(widget);

	GltkAllocation allocation = gltk_widget_get_allocation(widget);

	//render our overlay on top
	if (!button->isDown && !priv->animDrag && !priv->isConfig)
	{
		render_overlay(widget);
	}
	
	glBegin(GL_LINES);
	{
		glColor3fv(colorDark);
		glVertex2i(0, 0);
		glVertex2i(allocation.width, 0);
		glVertex2i(0, allocation.height);
		glVertex2i(allocation.width, allocation.height);
	}
	glEnd();

	//if (button->isDown)
	//{
	//	glBegin(GL_QUADS);
	//	{
	//		float offset = priv->slideOffset;

	//		if (offset < 0.1f && offset > -0.1f)
	//		{
	//			//left gradient
	//			glColor3fv(colorDark);
	//			glVertex2i(0, 0);
	//			glVertex2i(0, allocation.height);
	//			glColor3fv(colorHighlightBright);
	//			glVertex2i(allocation.width * (offset+0.1), allocation.height);
	//			glVertex2i(allocation.width * (offset+0.1), 0);

	//			//slide part
	//			glVertex2i(allocation.width * (offset+0.1), 0);
	//			glVertex2i(allocation.width * (offset+0.1), allocation.height);
	//			glVertex2i(allocation.width + allocation.width * (offset-0.1), allocation.height);
	//			glVertex2i(allocation.width + allocation.width * (offset-0.1), 0);

	//			//right gradient
	//			glVertex2i(allocation.width + allocation.width * (offset-0.1), 0);
	//			glVertex2i(allocation.width + allocation.width * (offset-0.1), allocation.height);
	//			glColor3fv(colorDark);
	//			glVertex2i(allocation.width, allocation.height);
	//			glVertex2i(allocation.width, 0);
	//		}
	//		else if (offset >= 0.1f)
	//		{
	//			//render slide part
	//			glColor3fv(colorDark);
	//			glVertex2i(0, allocation.height);
	//			glVertex2i(0, 0);
	//			glVertex2i(allocation.width * (offset-0.1), 0);
	//			glVertex2i(allocation.width * (offset-0.1), allocation.height);
	//			
	//			//render slide gradient
	//			glVertex2i(allocation.width * (offset-0.1), 0);
	//			glVertex2i(allocation.width * (offset-0.1), allocation.height);
	//			glColor3fv(colorHighlightBright);
	//			glVertex2i(allocation.width * (offset+0.1), allocation.height);
	//			glVertex2i(allocation.width * (offset+0.1), 0);

	//			//render selection
	//			glVertex2i(allocation.width, 0);
	//			glVertex2i(allocation.width * (offset+0.1), 0);
	//			glColor3fv(colorHighlightBright);
	//			glVertex2i(allocation.width * (offset+0.1), allocation.height);
	//			glVertex2i(allocation.width, allocation.height);

	//		}
	//		else // (offset <= -0.1f)
	//		{
	//			//render slide part
	//			glColor3fv(colorDark);
	//			glVertex2i(allocation.width, allocation.height);
	//			glVertex2i(allocation.width, 0);
	//			glVertex2i(allocation.width + allocation.width * (offset+0.1), 0);
	//			glVertex2i(allocation.width + allocation.width * (offset+0.1), allocation.height);
	//				
	//			//render slide gradient
	//			glVertex2i(allocation.width + allocation.width * (offset+0.1), 0);
	//			glVertex2i(allocation.width + allocation.width * (offset+0.1), allocation.height);
	//			glColor3fv(colorHighlightBright);
	//			glVertex2i(allocation.width + allocation.width * (offset-0.1), allocation.height);
	//			glVertex2i(allocation.width + allocation.width * (offset-0.1), 0);

	//			//render selection
	//			glVertex2i(allocation.width + allocation.width * (offset-0.1), 0);
	//			glVertex2i(0, 0);
	//			glColor3fv(colorHighlightBright);
	//			glVertex2i(0, allocation.height);
	//			glVertex2i(allocation.width + allocation.width * (offset-0.1), allocation.height);
	//		}
	//	}
	//	glEnd();

	//}

}

static void
gltk_config_button_render_overlay(GltkScreen* screen, GltkWidget* widget)
{
	USING_PRIVATE(widget);
	GltkButton* button = GLTK_BUTTON(widget);

	if ((!button->isDown && !priv->animDrag) || priv->isConfig)
		return;

	GltkAllocation allocation = gltk_widget_get_global_allocation(widget);

	//Create an alpha mask in the output buffer
	//glDisable(GL_BLEND);
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	//glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	//glRectf(allocation.x,
	//		allocation.y,
	//		allocation.x + allocation.width*2,
	//		allocation.y + allocation.height);
	//glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	//glRectf(allocation.x,
	//		allocation.y,
	//		allocation.x + allocation.width*1.4,
	//		allocation.y + allocation.height);
	//glBegin(GL_QUADS);
	//{
	//	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	//	glVertex2i(allocation.x + allocation.width*1.4, allocation.y);
	//	glVertex2i(allocation.x + allocation.width*1.4, allocation.y + allocation.height);
	//	glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
	//	glVertex2i(allocation.x + allocation.width*1.5, allocation.y + allocation.height);
	//	glVertex2i(allocation.x + allocation.width*1.5, allocation.y);
	//}
	//glEnd();
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	//glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	//glEnable(GL_BLEND);
	//TODO fade out

	glTranslatef(allocation.x + priv->slideOffset * allocation.width, allocation.y, 0.0f);
	render_overlay(widget);
	glTranslatef(-allocation.x - priv->slideOffset * allocation.width, -allocation.y, 0.0f);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static gboolean
anim_drag(GltkWidget* widget)
{
	USING_PRIVATE(widget);
	
	priv->slideOffset += MAX(0.01, priv->dOffset);
	priv->slideOffset = CLAMP(priv->slideOffset, 0.0, 1.6);
	gltk_widget_invalidate(widget);

	if (priv->slideOffset >= 1.5)
	{
		g_signal_emit(widget, signals[CONFIG_START], 0);
		priv->animDrag = 0;
		priv->isConfig = TRUE;
		g_object_notify_by_pspec(G_OBJECT(widget), properties[PROP_CONFIG_MODE]);
		return FALSE;
	}

	priv->dOffset *= 1.07;

	return TRUE;
}

static gboolean
gltk_config_button_touch_event(GltkWidget* widget, GltkEventTouch* touch)
{
	USING_PRIVATE(widget);

	if (priv->isConfig)
		return GLTK_WIDGET_CLASS(gltk_config_button_parent_class)->touch_event(widget, touch);

	if (touch->touchType == TOUCH_BEGIN)
	{
		priv->slideOffset = 0.0f;
		g_timer_start(priv->dragTimer);
	} 
	else if (touch->touchType == TOUCH_END)
	{
		if (!priv->animDrag && priv->slideOffset > 0.5f)
		{
			g_timer_stop(priv->dragTimer);
			priv->animDrag = g_timeout_add(CLAMP(g_timer_elapsed(priv->dragTimer, NULL) * 1000, 10, 100), 
					(GSourceFunc)anim_drag, widget);
		}
	}

	return GLTK_WIDGET_CLASS(gltk_config_button_parent_class)->touch_event(widget, touch);
}

static gboolean
gltk_config_button_drag_event(GltkWidget* widget, GltkEventDrag* event)
{
	USING_PRIVATE(widget);
	
	if (priv->isConfig)
		return GLTK_WIDGET_CLASS(gltk_config_button_parent_class)->drag_event(widget, event);

	if (event->longTouched || priv->animDrag)
		return FALSE;

	GltkAllocation allocation = gltk_widget_get_allocation(widget);

	priv->dOffset = (float)event->dx / allocation.width;
	priv->slideOffset += priv->dOffset;
	priv->slideOffset = CLAMP(priv->slideOffset, 0.0, 1.5);

	if (priv->slideOffset >= 1.0f)
	{
		g_timer_stop(priv->dragTimer);
		priv->animDrag = g_timeout_add(CLAMP(g_timer_elapsed(priv->dragTimer, NULL) * 1000, 10, 100), 
				(GSourceFunc)anim_drag, widget);
	}
	else
	{
		g_timer_start(priv->dragTimer);
	}
	gltk_widget_invalidate(widget);

	//don't consume the event
	return FALSE;
}

static gboolean
gltk_config_button_click_event(GltkWidget* widget, GltkEventClick* event)
{
	USING_PRIVATE(widget);

	//TODO: save changes

	priv->isConfig = FALSE;
	g_object_notify_by_pspec(G_OBJECT(widget), properties[PROP_CONFIG_MODE]);

	return FALSE;
}

static void
gltk_config_button_slide_event(GltkSlideButton* widget, gboolean dirRight)
{
	USING_PRIVATE(widget);

	if (dirRight)
	{
		//TODO: save changes (or not)
	}
	else
	{
		//TODO: revert changes
	}

	priv->isConfig = FALSE;
	g_object_notify_by_pspec(G_OBJECT(widget), properties[PROP_CONFIG_MODE]);

}


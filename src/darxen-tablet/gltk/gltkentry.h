/* gltkentry.h
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

#ifndef GLTKENTRY_H_DU9D6DGN
#define GLTKENTRY_H_DU9D6DGN

#include "gltkbutton.h"

G_BEGIN_DECLS

#define GLTK_ENTRY_ERROR gltk_entry_error_quark()

#define GLTK_TYPE_ENTRY				(gltk_entry_get_type())
#define GLTK_ENTRY(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), GLTK_TYPE_ENTRY, GltkEntry))
#define GLTK_ENTRY_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), GLTK_TYPE_ENTRY, GltkEntryClass))
#define GLTK_IS_ENTRY(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), GLTK_TYPE_ENTRY))
#define GLTK_IS_ENTRY_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), GLTK_TYPE_ENTRY))
#define GLTK_ENTRY_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GLTK_TYPE_ENTRY, GltkEntryClass))

typedef struct _GltkEntry			GltkEntry;
typedef struct _GltkEntryClass		GltkEntryClass;

struct _GltkEntry
{
	GltkButton parent;
};

struct _GltkEntryClass
{
	GltkButtonClass parent_class;
	
	/* signals */
	void		(*text_changed)		(GltkEntry* entry	);

	/* virtual funcs */
};

typedef enum
{
	GLTK_ENTRY_ERROR_FAILED
} GltkEntryError;

GType			gltk_entry_get_type	() G_GNUC_CONST;
GltkWidget*		gltk_entry_new		(const char* text);

const gchar*	gltk_entry_get_text	(GltkEntry* entry);
void			gltk_entry_set_text	(GltkEntry* entry, const gchar* text);

GQuark			gltk_entry_error_quark	();

G_END_DECLS

#endif


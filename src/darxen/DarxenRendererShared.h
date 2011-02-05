/* DarxenRenderer.h

   Copyright (C) 2008 Kevin Wells <kevin@darxen.org>

   This file is part of darxen.

   darxen is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   darxen is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with darxen.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __DARXEN_RENDERER_SHARED_H__
#define __DARXEN_RENDERER_SHARED_H__

#include "DarxenCommon.h"
#include "DarxenSettings.h"

#include "libdarxenRadarSites.h"

G_BEGIN_DECLS

typedef enum _DarxenTextLocation	DarxenTextLocation;

enum _DarxenTextLocation
{
	TEXT_UPPER_LEFT,
	TEXT_UPPER_RIGHT,
	TEXT_LOWER_LEFT,
	TEXT_LOWER_RIGHT
};

G_EXPORT void darxen_renderer_shared_init_text();
G_EXPORT void darxen_renderer_shared_render_text(const char *chrMessage, DarxenTextLocation location, gboolean blnBackground, float x, float y);
G_EXPORT void darxen_renderer_shared_render_text_line(const char *chrLine, DarxenTextLocation location, gboolean blnBackground, float x, float y);

G_END_DECLS

#endif

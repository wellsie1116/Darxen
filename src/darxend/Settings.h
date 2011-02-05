/* Settings.h
 *
 * Copyright (C) 2009 - Kevin Wells <kevin@darxen.org>
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



#ifndef SETTINGS_H
#define SETTINGS_H

#include <glib.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

void			settings_init		();
void			settings_cleanup	();

int				settings_get_current_year	();
const char*		settings_get_home_path		();
gchar*			settings_get_path			(const char* path);
void			settings_ensure_path		(const char* path);
gboolean		settings_path_exists		(const char* path);
gboolean		settings_file_exists		(const char* path);

char**			settings_get_path_vals		(const char* expr);

#endif /*SETTINGS_H*/

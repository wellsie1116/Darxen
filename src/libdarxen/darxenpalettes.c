/* darxenpalettes.c
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

#include "darxenpalettes.h"

#include <json-glib/json-glib.h>
#include <glib.h>

GHashTable* palettes = NULL;

const DarxenPalette*
darxen_palette_get_from_file(const char* path)
{
	if (!palettes)
		palettes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	DarxenPalette* res = g_hash_table_lookup(palettes, path);;
	if (!res)
	{
		GError* error = NULL;
		res = g_new(DarxenPalette, 1);

		JsonParser* parser = json_parser_new();
		if (!json_parser_load_from_file(parser, path, &error))
		{
			g_critical("Failed to load palette from %s", path);
			return NULL;
		}

		JsonArray* array = json_node_get_array(json_parser_get_root(parser));
		g_assert(array);
		g_assert(json_array_get_length(array) == 16);

		int i;
		for (i = 0; i < 16; i++)
		{
			JsonArray* color = json_array_get_array_element(array, i);
			g_assert(color);

			res->colors[i].r = json_array_get_int_element(color, 0) / 255.0;
			res->colors[i].g = json_array_get_int_element(color, 1) / 255.0;
			res->colors[i].b = json_array_get_int_element(color, 2) / 255.0;
			res->colors[i].a = json_array_get_length(color) == 4 ? json_array_get_int_element(color, 3) / 255.0 : 1.0f;
			//g_message("Loaded color: %f %f %f %f", res->colors[i].r, res->colors[i].g, res->colors[i].b, res->colors[i].a);
		}

		g_object_unref(G_OBJECT(parser));

		g_hash_table_insert(palettes, g_strdup(path), res);
	}

	return res;
}


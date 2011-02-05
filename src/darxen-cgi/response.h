/*
 * response.h
 *
 *  Created on: Jul 2, 2010
 *      Author: wellska1
 */

#ifndef RESPONSE_H_
#define RESPONSE_H_

#include <glib.h>

static GHashTable* headers = NULL;

static inline void
init_response()
{
	if (!headers)
		headers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}

void
response_header_set(const char* key, const char* value)
{
	init_response();

	g_hash_table_insert(headers, g_strdup(key), g_strdup(value));
}

void
response_write_headers()
{
	init_response();

	GHashTableIter it;
	g_hash_table_iter_init(&it, headers);
	char* key;
	char* value;
	while (g_hash_table_iter_next(&it, (gpointer)&key, (gpointer)&value))
		printf("%s: %s\n", key, value);
	printf("\n");
}

#endif /* RESPONSE_H_ */

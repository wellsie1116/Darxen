/*
 * request.c
 *
 *  Created on: Jul 2, 2010
 *      Author: wellska1
 */

#include "request.h"

#include <stdlib.h>
#include <string.h>

#include <glib.h>

static gchar* path = NULL;
static GHashTable* varTable = NULL;

static void init_vars()
{
	varTable = g_hash_table_new(g_str_hash, g_str_equal);

	char* tPath = strtok(g_strdup(g_getenv("URL")), "?");
	path = g_strdup(tPath);
	char* vars = strtok(NULL, "");
	gchar** varList = g_strsplit(vars, "&", -1);
	gchar** pVarList = varList;

	while (*pVarList)
	{
		char* k = g_strdup(strtok(*pVarList, "="));
		char* v = strtok(NULL, "");
		if (!v)
			v = strdup("");
		v = strdup(v);
		g_hash_table_insert(varTable, k, v);
		pVarList++;
	}
	g_strfreev(varList);
	g_free(tPath);
}

const char*
request_get_method()
{
	return g_getenv("HTTP_METHOD");
	//return "GET";
	//return "POST";
}

const char*
request_get_url_path()
{
	if (!path)
		init_vars();

	return path;
}

const char*
request_get_path_variable(const char* key)
{
	if (!varTable)
		init_vars();

	return g_hash_table_lookup(varTable, key);
}

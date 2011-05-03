/* Settings.c
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


#include "Settings.h"

#include <glib.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <sys/stat.h>

#include <string.h>

#include <time.h>

static xmlDoc* doc = NULL;
static xmlXPathContext* xpath = NULL;

void
settings_init()
{
	settings_ensure_path(NULL);
	if (!g_file_test(settings_get_path("Settings.xml"), G_FILE_TEST_EXISTS))
	{
		gchar* xml;
		g_message("Settings file not found, loading defaults");
		if (!g_file_get_contents("Defaults.xml", &xml, NULL, NULL))
		{
			g_warning("Default Settings failed to load");
			return;
		}
		if (!g_file_set_contents(settings_get_path("Settings.xml"), xml, -1, NULL))
			g_warning("Failed to write settings file");
		g_free(xml);
	}
	xmlInitParser();

    /* Load XML document */
    doc = xmlParseFile(settings_get_path("Settings.xml"));
    if (doc)
    {
    	/* Create xpath evaluation context */
	    xpath = xmlXPathNewContext(doc);
	    if (!xpath)
	    {
	    	xmlFreeDoc(doc);
	        g_warning("Failed to crawl settings file.");
	        doc = NULL;
	        xpath = NULL;
	    }
    }
    else
    {
    	g_warning("Failed to parse settings file.");
    	doc = NULL;
    	xpath = NULL;
    }

}

void
settings_cleanup()
{
	xmlCleanupParser();
}

int
settings_get_current_year()
{
	time_t rawtime = time(NULL);
	struct tm* utc = gmtime(&rawtime);
	int year = 1900 + utc->tm_year;
	return year;
}

const char*
settings_get_home_path()
{
	return settings_get_path(NULL);
}

gchar*
settings_get_path(const char* path)
{
	return g_build_filename(g_get_home_dir(), ".darxend", path, NULL);
}

void
settings_ensure_path(const char* path)
{
	gchar* fullpath = settings_get_path(path);
	if (!g_file_test(fullpath, G_FILE_TEST_IS_DIR))
		if (g_mkdir_with_parents(fullpath, S_IRWXU/* | S_IRWXG | S_IROTH | S_IXOTH*/) == -1)
			g_warning("The following path could not be created: %s", fullpath);
	g_free(fullpath);
}

gboolean
settings_path_exists(const char* path)
{
	gchar* fullpath = settings_get_path(path);
	gboolean result = g_file_test(fullpath, G_FILE_TEST_IS_DIR);
	if (!result)
		g_debug("Path does not exist: %s", fullpath);
	g_free(fullpath);
	return result;
}

gboolean
settings_file_exists(const char* path)
{
	gchar* fullpath = settings_get_path(path);
	gboolean result = g_file_test(fullpath, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR));
	g_free(fullpath);
	return result;
}

char**
settings_get_path_vals(const char* expr)
{
	xmlXPathObject* xpathObj = xmlXPathEvalExpression((unsigned char*)expr, xpath);
    if (!xpathObj)
    {
    	g_critical("Unable to evaluate xpath expression: %s", expr);
    	return NULL;
    }

    if (xpathObj->type != XPATH_NODESET)
    {
    	g_critical("Result is not a nodeset");
    	xmlXPathFreeObject(xpathObj);
    	return NULL;
    }
	int len = xpathObj->nodesetval ? xpathObj->nodesetval->nodeNr : 0;

    char** res = (char**)calloc(len + 1, sizeof(char*));
    int i;
    for (i = 0; i < len; i++)
    {
    	if (!xpathObj->nodesetval->nodeTab[i]->children)
			res[i] = strdup((char*)xpathObj->nodesetval->nodeTab[i]->content);
    	else
    		res[i] = strdup((char*)xpathObj->nodesetval->nodeTab[i]->children->content);
    }
    res[i] = NULL;

    xmlXPathFreeObject(xpathObj);
    return res;
}


/*********************
 * Private Functions *
 *********************/


/*
 * libdarxenFileSupport.c
 *
 *  Created on: Mar 14, 2010
 *      Author: wellska1
 */

#define G_LOG_DOMAIN "libdarxenFileSupport"
#include <glib/gmessages.h>

#include "libdarxenFileSupport.h"

#include <glib/gslist.h>
#include <glib/gutils.h>
#include <glib/gfileutils.h>
#include <glib/gstrfuncs.h>
#include <glib/gmem.h>

#include <malloc.h>
#include <string.h>

static gchar* appPath = NULL;
static gchar* overridableFilePath = NULL;

const char*
darxen_file_support_get_app_path()
{
	if (!appPath)
	{
		const gchar *chrHome = g_get_home_dir();
		gchar* programName = g_strdup_printf(".%s", g_get_prgname());
#ifdef G_OS_WIN32
		appPath = g_build_filename(chrHome, "Application Data", programName, NULL);
#else
		appPath = g_build_filename(chrHome, programName, NULL);
#endif
		g_free(programName);
	}
//	g_debug("get_app_path: %s", appPath);
	return appPath;
}

char*
darxen_file_support_get_overridable_file_path(const char *filename)
{
	if (overridableFilePath)
		g_free(overridableFilePath);

	overridableFilePath = g_build_filename(darxen_file_support_get_app_path(), filename, NULL);
	//g_debug("get_overridable_app_path: %s", overridableFilePath);
	if (g_file_test(overridableFilePath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
	{
		return overridableFilePath;
	}

	g_free(overridableFilePath);
	overridableFilePath = g_strdup(filename);
	//g_debug("get_overridable_app_path: %s", overridableFilePath);
	if (g_file_test(overridableFilePath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
	{
		return overridableFilePath;
	}

	g_free(overridableFilePath);

	const gchar* const* dataDirs = g_get_system_data_dirs();
	while (*dataDirs)
	{
		//TODO: PACKAGE instead of "darxen"
		overridableFilePath = g_build_filename(*dataDirs, "darxen", g_get_prgname(), filename, NULL);
		//g_debug("get_overridable_app_path: %s", overridableFilePath);
		if (g_file_test(overridableFilePath, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))
		{
			return overridableFilePath;
		}
		g_free(overridableFilePath);

		dataDirs++;
	}

	overridableFilePath = NULL;
	//g_debug("get_overridable_app_path: %s", "null");
	return NULL;
}

char*
darxen_file_support_find_path(const char *first, ...)
{
	va_list args;
	char* arg;
	int argc = 1;

	va_start(args, first);

	GSList* argList = g_slist_append(NULL, (char*)first);
	GSList* pArgList = argList;

	while ((arg = va_arg(args, char*)))
	{
		argList = g_slist_append(argList, arg);
		argc++;
	}
	va_end(args);

	char** argv = (char**)calloc(argc + 1, sizeof(char*));
	argv[argc] = NULL;

	int i = 0;
	while (pArgList)
	{
		argv[i] = (char*)pArgList->data;
		i++;
		pArgList = pArgList->next;
	}
	g_slist_free(argList);

	gchar* path = g_build_filenamev(argv);
	char* res = darxen_file_support_get_overridable_file_path(path);
	g_free(path);

	return res;
}


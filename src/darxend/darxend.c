/* darxend.cc
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



#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <curl/curl.h>

#include "restapi.h"
#include "Settings.h"
#include "ClientManager.h"
#include "RadarDataManager.h"

#ifdef G_OS_WIN32
#define nanosleep(req, rem) pthread_delay_np(req)
#endif

/* TODO:
 * there are memory leaks, there are unsafe memory practices
 * Not threadsafe, yet we use threads!
 */

#define BACKLOG 100

static gboolean
main_loop_keepalive(void* user_data)
{
	return TRUE;
}

int
main (int argc, char *argv[])
{
	/*Settings* settings = Settings::getInstance();
	printf("%s\n", settings->getString("/Settings/Sites/Site[@id=\"KILX\"]/Product"));
	Settings::PathIterator* it = settings->getIterator("/Settings/Sites/Site[@id=\"KLOT\"]/Product");
	while (it->hasNext())
	{
		it->moveNext();
		printf("S: %s\n", it->getString());
	}*/

	settings_init();

	GMainLoop* loop;

	g_type_init();

	curl_global_init(CURL_GLOBAL_WIN32);

	loop = g_main_loop_new(NULL, FALSE);
	g_timeout_add_seconds_full(G_PRIORITY_LOW, 2, main_loop_keepalive, NULL, NULL);

	settings_ensure_path("archives/level3");

	radar_data_manager_init();

	char** sites = settings_get_path_vals("/Settings/Pollers/Site/@id");
	char** psites = sites;
	while (*psites)
	{
		gchar* site = g_ascii_strdown(*psites, -1);
		gchar* sitepath = g_strdup_printf("/Settings/Pollers/Site[@id=\"%s\"]/Product/text()", site);
		char** products = settings_get_path_vals(sitepath);
		char** pproducts = products;
		while (*pproducts)
		{
			char* product = *pproducts;
			printf("Adding handler for %s %s\n", site, product);
			product = g_ascii_strup(product, -1);
			radar_data_manager_add_poller(NULL, site, product);
			g_free(product);
			free(*pproducts);
			pproducts++;
		}
		free(products);
		free(*psites);
		g_free(sitepath);
		g_free(site);
		psites++;
	}
	free(sites);

	client_manager_init();
	
	printf("Launching REST API server...");
	if (restapi_init())
		printf("FAILED\n");
	else
		printf("OK\n");

	printf("starting main loop\n");
	g_main_loop_run(loop);

	printf("Main loop killed\n");
	restapi_shutdown();
	settings_cleanup();

	return 0;
}


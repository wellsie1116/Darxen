/* TestRestfulClient.c
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

#include "TestingCommon.h"

#include "libdarxenRestfulClient.h"

#include <stdlib.h>

typedef struct {
	GPid darxend;
	DarxenRestfulClient* client;
} RestfulClientFixture;

static void test_construct_setup(RestfulClientFixture* fix, gconstpointer testdata)
{
	fix->client = darxen_restful_client_new();

	//gchar* args[] = {"darxend", NULL};
	//g_assert(g_spawn_async("../darxend", args, NULL, 0, NULL, NULL, &fix->darxend, NULL));
	//sleep(1);
}

static void test_construct_teardown(RestfulClientFixture* fix, gconstpointer testdata)
{
	g_object_unref(G_OBJECT(fix->client));

	//g_spawn_close_pid(fix->darxend);
}

static void test_setup(RestfulClientFixture* fix, gconstpointer testdata)
{
	GError* error = NULL;

	fix->client = darxen_restful_client_new();
	int res = darxen_restful_client_connect(fix->client, &error);
	g_assert_no_error(error);
	g_assert_cmpint(res, >, 0);
}

static void test_teardown(RestfulClientFixture* fix, gconstpointer testdata)
{
	GError* error = NULL;
	int res;

	res = darxen_restful_client_disconnect(fix->client, &error);
	g_assert_no_error(error);
	g_assert_cmpint(res, ==, 0);
	g_object_unref(G_OBJECT(fix->client));
}

static void test_construct(RestfulClientFixture* fix, gconstpointer testdata) 
{
	g_assert(fix->client);
}

static void test_connection(RestfulClientFixture* fix, gconstpointer testdata)
{
	GError* error = NULL;

	int res = darxen_restful_client_connect(fix->client, &error);
	g_assert_no_error(error);
	g_assert_cmpint(res, >, 0);
	g_message("Client ID: %d", res);

	res = darxen_restful_client_disconnect(fix->client, &error);
	g_assert_no_error(error);
	g_assert_cmpint(res, ==, 0);
}

static void test_pollers(RestfulClientFixture* fix, gconstpointer testdata)
{
	int res;
	DarxenPoller* poller;
	GError* error = NULL;
	
	RadarPoller* pollers;
	int pollerCount;
	
	pollers = darxen_restful_client_list_pollers(fix->client, &pollerCount, &error);
	g_assert_no_error(error);
	g_assert(pollers);
	g_assert_cmpint(pollerCount, ==, 0);
	g_assert(!pollers[0].site && !pollers[0].product);
	free(pollers);

	poller = darxen_restful_client_add_poller(fix->client, "klot", "N0R", &error);
	g_assert_no_error(error);
	g_assert(DARXEN_IS_POLLER(poller));
	
	pollers = darxen_restful_client_list_pollers(fix->client, &pollerCount, &error);
	g_assert_no_error(error);
	g_assert(pollers);
	g_assert_cmpint(pollerCount, ==, 1);
	g_assert_cmpstr(pollers[0].site, ==, "klot");
	g_assert_cmpstr(pollers[0].product, ==, "N0R");
	free(pollers);

	poller = darxen_restful_client_add_poller(fix->client, "klot", "N1R", &error);
	g_assert_no_error(error);
	g_assert(DARXEN_IS_POLLER(poller));
	
	poller = darxen_restful_client_add_poller(fix->client, "kilx", "N0R", &error);
	g_assert_no_error(error);
	g_assert(DARXEN_IS_POLLER(poller));

	res = darxen_restful_client_remove_poller(fix->client, "klot", "N0R", &error);
	g_assert_no_error(error);
	g_assert_cmpint(res, ==, 0);

	res = darxen_restful_client_remove_poller(fix->client, "klot", "N1R", &error);
	g_assert_no_error(error);
	g_assert_cmpint(res, ==, 0);

	pollers = darxen_restful_client_list_pollers(fix->client, &pollerCount, &error);
	g_assert_no_error(error);
	g_assert(pollers);
	g_assert_cmpint(pollerCount, ==, 1);
	g_assert_cmpstr(pollers[0].site, ==, "kilx");
	g_assert_cmpstr(pollers[0].product, ==, "N0R");
	free(pollers);
	
	res = darxen_restful_client_remove_poller(fix->client, "kilx", "N0R", &error);
	g_assert_no_error(error);
	g_assert_cmpint(res, ==, 0);

	pollers = darxen_restful_client_list_pollers(fix->client, &pollerCount, &error);
	g_assert_no_error(error);
	g_assert(pollers);
	g_assert_cmpint(pollerCount, ==, 0);
	g_assert(!pollers[0].site && !pollers[0].product);
	free(pollers);
}

void
testing_restful_client_init()
{
	g_test_add("/libdarxen/RestfulClient/construct", RestfulClientFixture, NULL,
		   test_construct_setup, test_construct, test_construct_teardown);
	g_test_add("/libdarxen/RestfulClient/connection", RestfulClientFixture, NULL,
		   test_construct_setup, test_connection, test_construct_teardown);

	g_test_add("/libdarxen/RestfulClient/pollers", RestfulClientFixture, NULL,
		   test_setup, test_pollers, test_teardown);
}


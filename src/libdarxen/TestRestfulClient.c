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

typedef struct {
	DarxenRestfulClient* client;
} RestfulClientFixture;


static void test_setup(RestfulClientFixture* fix, gconstpointer testdata) {
	fix->client = darxen_restful_client_new();
}

static void test_teardown(RestfulClientFixture* fix, gconstpointer testdata) {
	g_object_unref(G_OBJECT(fix->client));
}

static void test_construct(RestfulClientFixture* fix, gconstpointer testdata) {
	g_assert(fix->client);
}

static void test_connect(RestfulClientFixture* fix, gconstpointer testdata) {
	GError* error = NULL;
	int res = darxen_restful_client_connect(fix->client, &error);
	g_assert_no_error(error);
	g_assert_cmpint(res, >, 0);
	g_message("Client ID: %d", res);
}

static void test_disconnect(RestfulClientFixture* fix, gconstpointer testdata) {
	GError* error = NULL;
	int res = darxen_restful_client_connect(fix->client, &error);
	g_assert_no_error(error);
	g_assert_cmpint(res, >, 0);
	res = darxen_restful_client_disconnect(fix->client, &error);
	g_assert_no_error(error);
	g_assert_cmpint(res, ==, 0);
}

void
testing_restful_client_init()
{
	g_test_add("/libdarxen/RestfulClient/construct", RestfulClientFixture, NULL, test_setup, test_construct, test_teardown);
	g_test_add("/libdarxen/RestfulClient/connect", RestfulClientFixture, NULL, test_setup, test_connect, test_teardown);
	g_test_add("/libdarxen/RestfulClient/disconnect", RestfulClientFixture, NULL, test_setup, test_disconnect, test_teardown);
}


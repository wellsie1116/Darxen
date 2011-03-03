/* darxencli.c
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

#include "libdarxenClient.h"
#include "DarxenParserLevel3.h"

#include <glib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "request.h"
#include "response.h"

#include "DarxenParserLevel3.h"

/*
static gboolean parse_date(const gchar *option_name, const gchar *value, gpointer data, GError **error);

static void poll_client_data_received(DarxenClient* client, DarxenClientDataPacket* data, gpointer user_data);
*/

static void client_data_received(DarxenClient* client, DarxenClientDataPacket* data, gpointer user_data);

typedef enum
{
	TEST_BASIC,
	TEST_ADDITION,
	TEST_SUBTRACTION
} TestType;
void init_test(TestType type)
{
	putenv("HTTP_METHOD=GET");

	switch (type)
	{
	case TEST_BASIC:
		putenv("URL=asdf/index.htm?a=1&b=2&c");
		break;
	case TEST_ADDITION:
		putenv("URL=math/add?x=1.123456&y=10");
		break;
	case TEST_SUBTRACTION:
		putenv("URL=math/subtract?x=15.3&y=15.0");
		break;
	default:
		printf("Unsupported test type\n");
		break;
	}
}

int
main2(int argc, char** argv)
{
	GMainLoop* loop = g_main_loop_new(NULL, FALSE);
	GError* error = NULL;

	DarxenClient* client = NULL; //darxen_client_new("localhost", FALSE);

	g_signal_connect(client, "data-received", (GCallback)client_data_received, NULL);

	// if (!darxen_client_connect(client, &error))
	// {
	// 	//TODO: error
	// }

	const char* site = request_get_path_variable("site");
	const char* product = request_get_path_variable("prod");
	const char* frames = request_get_path_variable("count");
	int frameCount = 1;
	if (frames)
		frameCount = MAX(1, MIN(40, atoi(frames)));


	// if (!darxen_client_request_frames(client, site, product, frameCount, &error))
	// //if (!darxen_client_register_poller(client, (char*)site, (char*)product, &error))
	// {
	// 	//TODO: error
	// }

	g_main_loop_run(loop);


	return 0;
}

static void
client_data_received(DarxenClient* client, DarxenClientDataPacket* data, gpointer user_data)
{
	fprintf(stderr, "Received data packet\n");
}



int
main(int argc, char** argv)
{

	/*

//	char* str = strdup("a b c");
//	char* a = strtok(str, " ");
//	char* b = strtok(NULL, " ");
//	char* c = strtok(NULL, " ");
//	char* d = strtok(NULL, " ");

	init_test(TEST_BASIC);

	char* path = request_get_url_path();
	char* a = request_get_path_variable("a");
	char* b = request_get_path_variable("b");
	char* c = request_get_path_variable("c");
	char* d = request_get_path_variable("d");

	printf("Path: %s\n", path ? path : "NULL");
	printf("A: %s\n", a ? a : "NULL");
	printf("B: %s\n", b ? b : "NULL");
	printf("C: %s\n", c ? c : "NULL");
	printf("D: %s\n", d ? d : "NULL");
	printf("Method: %s\n", request_get_method());

	*/

	init_test(TEST_ADDITION);
	init_test(TEST_SUBTRACTION);

	response_header_set("Connection", "close");
	response_header_set("Content-Type", "text/plain");

	const char* cx = request_get_path_variable("x");
	const char* cy = request_get_path_variable("y");
	if (!cx || !cy)
	{
		response_write_headers();
		printf("Required param (x or y) missing\n");
		return 1;
	}

	double x = atof(cx);
	double y = atof(cy);

	double res;
	const char* path = request_get_url_path();
	if (!strcmp("math/add", path))
	{
		res = x + y;
	}
	else if (!strcmp("math/subtract", path))
	{
		res = x - y;
	}
	else
	{
		response_write_headers();
		printf("Invalid url: %s\n", path);
		return 1;
	}


	response_write_headers();

	printf("Res: %.3f\n", res);

	return 0;
}


/*

static int
main_poll()
{
	GError* error = NULL;

	DarxenClient* client = darxen_client_new("http://127.0.0.1:4888/DarxenService", FALSE);

	if (!darxen_client_connect(client, &error))
	{
		g_critical("Connect Failed: %s", error->message);
		g_object_unref(client);
		return 1;
	}

	gchar** pchrProducts = chrProducts;
	while (*pchrProducts)
	{
		printf("Registering %s/%s\n", chrSite, *pchrProducts);
		if (!darxen_client_register_poller(client, chrSite, *pchrProducts, &error))
		{
			g_critical("Failed to add poller: %s", error->message);
		}

		pchrProducts++;
	}

	printf("Initialized OK, waiting for response\n");
	fflush(stdout);

	GMainLoop* loop = g_main_loop_new(NULL, FALSE);

	g_signal_connect(client, "data-received", G_CALLBACK(poll_client_data_received), NULL);

	g_main_loop_run(loop);

	return 0;
}

static void
poll_client_data_received(DarxenClient* client, DarxenClientDataPacket* data, gpointer user_data)
{
	printf("Data received from %s/%s\n", data->site, data->product);fflush(stdout);

	int len;
	guchar* fdata = g_base64_decode(data->data, &len);

	FILE* f = tmpfile();

	fwrite(fdata, 1, len, f);
	fseek(f, 0, SEEK_SET);

	ProductsLevel3Data* parsed;

	if (!(parsed = parser_lvl3_parse_file(f)))
	{
		g_critical("Failed to parse level 3 data");
		return;
	}

	printf("Header: %s\n", parsed->chrWmoHeader);

	fflush(stdout);
}
*/

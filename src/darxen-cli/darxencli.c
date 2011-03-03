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

static gchar* chrOp = NULL;
static gchar* argv2 = NULL;

static gchar* chrSite = NULL;
static gchar** chrProducts = NULL;

static gchar* chrProduct = NULL;
static GDate* dateStart = NULL;
static int timeStart = -1;
static GDate* dateEnd = NULL;
static int timeEnd = -1;


static gboolean parse_date(const gchar *option_name, const gchar *value, gpointer data, GError **error);

static int main_search();
static int main_poll();

static void poll_client_data_received(DarxenClient* client, DarxenClientDataPacket* data, gpointer user_data);

static GOptionEntry entriesMain[] =
{
/*	long name			short name	flags	argument type					variable		description												argument description */
	{"op",						'o', 	0, G_OPTION_ARG_STRING,				&chrOp,			"The operator to perform",		NULL},
	{G_OPTION_REMAINING,	  '\0', 	0, G_OPTION_ARG_STRING_ARRAY,		&argv2,			NULL,							NULL},
	{NULL}
};

//darxen-cli search -s klot -p N0R --startdate=2/8/2010-13:10 --enddate=2/8/2010-15:00
static GOptionEntry entriesSearch[] =
{
/*	long name		short name	flags	argument type		variable		description												argument description */
	{"site",				's', 0, G_OPTION_ARG_STRING,	&chrSite,		"The radar site to search from",						NULL},
	{"product",				'p', 0, G_OPTION_ARG_STRING,	&chrProduct,	"The product to search from site",						NULL},
	{"startdate",			'a', 0, G_OPTION_ARG_CALLBACK,	parse_date,		"The start date",										NULL},
	{"enddate",				'b', 0, G_OPTION_ARG_CALLBACK,	parse_date,		"The end date",											NULL},
	{NULL}
};

//darxen-cli poll -s klot -p N0R -p N1R
static GOptionEntry entriesPoll[] =
{
/*	long name		short name	flags	argument type			variable		description												argument description */
	{"site",				's', 0, G_OPTION_ARG_STRING,		&chrSite,		"The radar site to poll from",							NULL},
	{"product",			'p', 0, G_OPTION_ARG_STRING_ARRAY,	&chrProducts,	"The products to poll from site",						NULL},
	{NULL}
};

//help
//render
//disp


int
main (int argc, char *argv[])
{
	GError* error = NULL;
	GOptionContext *context;
	GOptionGroup* groupMain;
	GOptionGroup* groupSearch;
	GOptionGroup* groupPoll;


	g_type_init();

	context = g_option_context_new("\nArchive, Download, Parse, and render NEXRAD data");

	groupMain = g_option_group_new("Main", "", "", NULL, NULL);
	g_option_group_add_entries(groupMain, entriesMain);
	g_option_context_add_group(context, groupMain);
	g_option_context_set_ignore_unknown_options(context, TRUE);

	if (!g_option_context_parse(context, &argc, &argv, &error))
	{
		g_error("Failed to parse cli options (phase 1): %s", error->message);
		return 1;
	}

	if (!chrOp)
	{
		g_error("Operation required");
		return 1;
	}

	g_option_context_set_ignore_unknown_options(context, FALSE);

	groupSearch = g_option_group_new("Search", "Search through archived data", "Search through archived data", NULL, NULL);
	g_option_group_add_entries(groupSearch, entriesSearch);

	groupPoll = g_option_group_new("Poll", "Poll data from darxend", "Continually polls for data, archiving it in the process", NULL, NULL);
	g_option_group_add_entries(groupPoll, entriesPoll);

	if (!strcasecmp(chrOp, "search"))
	{
		g_option_context_add_group(context, groupSearch);
	}
	else if (!strcasecmp(chrOp, "poll"))
	{
		g_option_context_add_group(context, groupPoll);
	}
//	else if (!strcasecmp(chrOp, "render"))
//	{
//
//	}
//	else if (!strcasecmp(chrOp, "disp"))
//	{
//
//	}
	else
	{
		printf("Invalid operation: %s\n", chrOp);
	}

	if (!g_option_context_parse(context, &argc, &argv, &error))
	{
		g_error("Failed to parse cli options(phase 2): %s", error->message);
		return 1;
	}

	int res;

	if (!strcasecmp(chrOp, "search"))
	{
		res = main_search();
	}
	else if (!strcasecmp(chrOp, "poll"))
	{
		res = main_poll();
	}
//	else if (!strcasecmp(chrOp, "render"))
//	{
//
//	}
//	else if (!strcasecmp(chrOp, "disp"))
//	{
//
//	}

	//g_option_context_add_main_entries(context, entries, NULL);
	//g_option_context_set_ignore_unknown_options(context, TRUE);

	g_option_group_free(groupSearch);
	g_option_group_free(groupPoll);
	g_option_context_free(context);

	return res;
}

static gboolean
parse_date(const gchar *option_name, const gchar *value, gpointer data, GError **error)
{
	int month;
	int day;
	int year;
	int hour;
	int minute;
	if (sscanf(value, "%i/%i/%i-%i:%i", &month, &day, &year, &hour, &minute) != 5)
	{
		g_set_error(error, G_OPTION_ERROR, G_OPTION_ERROR_FAILED, "Failed to parse date/time object");
		return FALSE;
	}

	GDate* date = g_date_new_dmy(day, month, year);
	int time = hour * 60 * 60 + minute * 60;

	if (!g_ascii_strcasecmp(option_name, "-a") || !g_ascii_strcasecmp(option_name, "--startdate"))
	{
		dateStart = date;
		timeStart = time;
	}
	else
	{
		dateEnd = date;
		timeEnd = time;
	}

	return TRUE;
}

static int
main_search()
{

	return 0;
}

static int
main_poll()
{
	GError* error = NULL;

	DarxenClient* client = NULL; //darxen_client_new("http://127.0.0.1:4888/DarxenService", FALSE);

	//if (!darxen_client_connect(client, &error))
	//{
	//	g_critical("Connect Failed: %s", error->message);
	//	g_object_unref(client);
	//	return 1;
	//}

	gchar** pchrProducts = chrProducts;
	while (*pchrProducts)
	{
		printf("Registering %s/%s\n", chrSite, *pchrProducts);
		//if (!darxen_client_register_poller(client, chrSite, *pchrProducts, &error))
		//{
		//	g_critical("Failed to add poller: %s", error->message);
		//}

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

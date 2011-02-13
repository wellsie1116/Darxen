/* FtpConnection.c
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


#include "FtpConnection.h"

#include "FtpDirList.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#define TEST_SERVER 0

#if TEST_SERVER == 1
#define HOSTNAME "ftp://localhost"
#else
#define HOSTNAME "ftp://tgftp.nws.noaa.gov"
#endif

G_DEFINE_TYPE(DarxenFtpConnection, darxen_ftp_connection, G_TYPE_OBJECT)
static void darxen_ftp_connection_finalize(GObject* gobject);

#define USING_PRIVATE(obj) DarxenFtpConnectionPrivate* priv = DARXEN_FTP_CONNECTION_GET_PRIVATE(obj);
#define DARXEN_FTP_CONNECTION_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_FTP_CONNECTION, DarxenFtpConnectionPrivate))

typedef struct _DarxenFtpConnectionPrivate		DarxenFtpConnectionPrivate;
struct _DarxenFtpConnectionPrivate
{
	CURL* curl;
	char* site;
	const char* product;
};

typedef struct
{
	char *chrMem;
	size_t intSize;
} FtpMem;

typedef struct
{
	const char* WMO;
	const char* RPG; //ftp
} ProductCode;

static ProductCode products[] =
{
	//Base Reflectivity
	{"N0R", "DS.p19r0"},
	{"N1R", "DS.p19r1"},
	{"N2R", "DS.p19r2"},
	{"N3R", "DS.p19r3"},

	//Base Radial Velocity
	{"N0S", "DS.p27v0"},
	{"N1S", "DS.p27v1"},
	{"N2S", "DS.p27v2"},
	{"N3S", "DS.p27v3"},

	//Echo Tops
	{"NET", "DS.p41et"},

	//Vertical Integrated Liquid
	{"NVL", "DS.57vil"},

	{NULL, NULL}
};

static size_t ftp_mem_read(void *ptr, size_t size, size_t nmemb, void *stream);

static void
darxen_ftp_connection_class_init(DarxenFtpConnectionClass* klass)
{
	g_type_class_add_private(klass, sizeof(DarxenFtpConnectionPrivate));

	klass->parent_class.finalize = darxen_ftp_connection_finalize;
}

static void
darxen_ftp_connection_init(DarxenFtpConnection* self)
{
	USING_PRIVATE(self)

	priv->site = NULL;
	priv->product = NULL;
}

static void
darxen_ftp_connection_finalize(GObject* gobject)
{
	DarxenFtpConnection* self = DARXEN_FTP_CONNECTION(gobject);
	USING_PRIVATE(self);

	if (priv->site)
		g_free(priv->site);
	curl_easy_cleanup(priv->curl);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(darxen_ftp_connection_parent_class)->finalize(gobject);

}

DarxenFtpConnection*
darxen_ftp_connection_new()
{
	GObject *gobject = (GObject*)g_object_new(DARXEN_TYPE_FTP_CONNECTION, NULL);
	DarxenFtpConnection* self = DARXEN_FTP_CONNECTION(gobject);

	USING_PRIVATE(self)


	priv->curl = curl_easy_init();
	curl_easy_setopt(priv->curl, CURLOPT_URL, HOSTNAME);
	curl_easy_setopt(priv->curl, CURLOPT_WRITEDATA, self);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

	return (DarxenFtpConnection*)gobject;
}

GQuark
darxen_ftp_connection_error_quark()
{
	return g_quark_from_static_string("darxen-ftp_connection-error-quark");
}

/*********************
 * Private Functions *
 *********************/

void
darxen_ftp_connection_set_site(DarxenFtpConnection* self, char* site)
{
	USING_PRIVATE(self)
	if (priv->site)
		g_free(priv->site);
	gchar* siteLower = g_ascii_strdown(site, -1);
	priv->site = g_strdup_printf("SI.%s", siteLower);
	g_free(siteLower);
}

gboolean
darxen_ftp_connection_set_product(DarxenFtpConnection* self, char* product)
{
	USING_PRIVATE(self)

	int i;
	for (i = 0; products[i].WMO; i++)
	{
		if (!strcmp(products[i].WMO, product))
		{
			priv->product = products[i].RPG;
			return TRUE;
		}
	}
	return FALSE;

}

DarxendFtpEntry**
darxen_ftp_connection_get_latest_entries(DarxenFtpConnection* self)
{
	USING_PRIVATE(self)

	g_assert(priv->site);
	g_assert(priv->product);

	gchar* url = g_build_path("/", HOSTNAME, "SL.us008001/DF.of/DC.radar", priv->product, priv->site, "/", NULL);
	FtpMem data;
	data.intSize = 0;
	data.chrMem = NULL;
//	curl_easy_setopt(priv->curl, CURLOPT_VERBOSE, url);
	curl_easy_setopt(priv->curl, CURLOPT_URL, url);
	curl_easy_setopt(priv->curl, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(priv->curl, CURLOPT_WRITEFUNCTION, ftp_mem_read);

	curl_easy_perform(priv->curl);

	if (data.intSize == 0)
	{
		g_warning("Server did not return a directory listing for path: %s", url);
		g_free(url);
		return NULL;
	}

	g_free(url);

	DarxendFtpDirList* listing = darxend_ftp_dir_list_new(data.chrMem);
	free(data.chrMem);

	//FIXME: the number of entries to read should not be set here
	int count = 5;
	DarxendFtpEntry** result = darxend_ftp_dir_list_get_latest_entries(listing, count);

	g_object_unref(listing);

	return result;
}

char*
darxen_ftp_connection_read_entry(DarxenFtpConnection* self, DarxendFtpEntry* entry, int* size)
{
	USING_PRIVATE(self)

	g_assert(entry);

	gchar* url = g_build_path("/", HOSTNAME, "SL.us008001/DF.of/DC.radar", priv->product, priv->site, entry->chrFileName, NULL);
	FtpMem data;
	data.intSize = 0;
	data.chrMem = NULL;
	g_message("DOWNLOADING URL: %s", url);
//	curl_easy_setopt(priv->curl, CURLOPT_VERBOSE, url);
	curl_easy_setopt(priv->curl, CURLOPT_URL, url);
	curl_easy_setopt(priv->curl, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(priv->curl, CURLOPT_WRITEFUNCTION, ftp_mem_read);

	curl_easy_perform(priv->curl);
	g_free(url);

	*size = data.intSize;
	return data.chrMem;
}

/*void
FtpConnection::ls()
{
	gchar* url = g_build_path("/", HOSTNAME, "SL.us008001/DF.of/DC.radar", product, site, "/", NULL);
	printf("URL: %s\n", url);
	FtpMem data;
	data.intSize = 0;
	data.chrMem = NULL;
	curl_easy_setopt(curl, CURLOPT_VERBOSE, url);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FtpConnection::ftp_mem_read);
	//curl_easy_setopt(curl, CURLOPT_FILETIME, 1);
	//curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "NOOP");
	//CURLOPT_TIMECONDITION - CURL_TIMECOND_IFMODSINCE
	//CURLOPT_TIMEVALUE

	//curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 1);

	//curl_slist *slist=NULL;
	//slist = curl_slist_append(slist, "MDTM sn.last");
	//curl_easy_setopt(curl, CURLOPT_POSTQUOTE, slist);

	curl_easy_perform(curl);
	DarxendFtpDirList* listing = darxend_ftp_dir_list_new(data.chrMem);
	//curl_slist_free_all(slist);
	g_free(url);
	printf("Directory listing:\n%s\n", data.chrMem);

	int count = 5;
	DarxendFtpEntry** latest = darxend_ftp_dir_list_get_latest_entries(listing, count);
	if (latest == NULL)
	{
		printf("Latest entries were null\n");
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			const DarxendFtpEntry* entry = latest[i];
			printf("Entry %i: %s\n", i + 1, entry->chrFileName);
		}
		g_object_unref(listing);
	}
}*/

size_t
ftp_mem_read(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t intSize = size * nmemb;
	FtpMem *objMem = (FtpMem*)stream;
	if (!objMem)
		return 0;
	objMem->chrMem = (char*)realloc(objMem->chrMem, objMem->intSize + intSize + 1);
	if (!objMem->chrMem)
		return 0;
	memcpy(&(objMem->chrMem[objMem->intSize]), ptr, intSize);
	objMem->intSize += intSize;
	objMem->chrMem[objMem->intSize] = 0x00;
	return intSize;
}


/* FtpDirList.c
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

#include "FtpDirList.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

static void darxend_ftp_dir_list_finalize(GObject* self);

static gboolean isDateBGreater(DarxendFtpEntry* a, DarxendFtpEntry* b);
static gint dir_sorter(gconstpointer a, gconstpointer b);

static char chrMonths[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
								"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

G_DEFINE_TYPE(DarxendFtpDirList, darxend_ftp_dir_list, G_TYPE_OBJECT)

#define USING_PRIVATE(obj) DarxendFtpDirListPrivate* priv = DARXEND_FTP_DIR_LIST_GET_PRIVATE(obj);
#define DARXEND_FTP_DIR_LIST_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEND_TYPE_FTP_DIR_LIST, DarxendFtpDirListPrivate))

typedef struct _DarxendFtpDirListPrivate		DarxendFtpDirListPrivate;
struct _DarxendFtpDirListPrivate
{
	gchar* dirlist;
	GList* entries; //DarxendFtpEntry
};

static void
darxend_ftp_dir_list_class_init(DarxendFtpDirListClass* klass)
{
	g_type_class_add_private(klass, sizeof(DarxendFtpDirListPrivate));

	klass->parent_class.finalize = darxend_ftp_dir_list_finalize;
}

static void
darxend_ftp_dir_list_init(DarxendFtpDirList* self)
{
	USING_PRIVATE(self)

	priv->dirlist = NULL;
	priv->entries = NULL;
}

static void
darxend_ftp_dir_list_finalize(GObject* gobject)
{
	DarxendFtpDirList* self = DARXEND_FTP_DIR_LIST(gobject);
	USING_PRIVATE(self);

	GList* pentries = priv->entries;
	while (pentries)
	{
		free(pentries->data);
		pentries = pentries->next;
	}
	g_list_free(priv->entries);
	g_free(priv->dirlist);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(darxend_ftp_dir_list_parent_class)->finalize(gobject);

}

/********************
 * Public Functions *
 ********************/

DarxendFtpDirList*
darxend_ftp_dir_list_new(char* dirlist)
{
	GObject* gobject = (GObject*)g_object_new(DARXEND_TYPE_FTP_DIR_LIST, NULL);
	DarxendFtpDirList* self = DARXEND_FTP_DIR_LIST(gobject);

	USING_PRIVATE(self)

	priv->dirlist = g_strdup(dirlist);

	DarxendFtpEntry* entry;
	char* chrPermissions = strtok(priv->dirlist, " "); //permissions
	while (chrPermissions)
	{
		entry = (DarxendFtpEntry*)malloc(sizeof(DarxendFtpEntry));
		strtok(NULL, " "); //?
		strtok(NULL, " "); //User/Group
		strtok(NULL, " "); //User/Group
		strtok(NULL, " "); //filesize
		char* chrMonth = strtok(NULL, " ");
		int i;
		for (i = 0; i < 12; i++)
			if (!strcmp(chrMonth, chrMonths[i]))
				entry->month = i + 1;
		entry->day = atoi(strtok(NULL, " "));
		entry->hour = atoi(strtok(NULL, ":"));
		entry->min = atoi(strtok(NULL, " "));
		entry->chrFileName = strtok(NULL, "\n");
		while (g_str_has_suffix(entry->chrFileName, "\r"))
			entry->chrFileName[strlen(entry->chrFileName) - 1] = '\0';
		priv->entries = g_list_append(priv->entries, entry);
		chrPermissions = strtok(NULL, " ");
	}
	priv->entries = g_list_sort(priv->entries, dir_sorter);
//	GList* pentries = this->entries;
//	while (pentries)
//	{
//		entry = (FtpEntry*)pentries->data;
//		printf("%i/%i %i:%i\n", entry->month, entry->day, entry->hour, entry->min);
//		pentries = pentries->next;
//	}

	return (DarxendFtpDirList*)gobject;
}

GQuark
darxend_ftp_dir_list_error_quark()
{
	return g_quark_from_static_string("darxend-ftp_dir_list-error-quark");
}

const DarxendFtpEntry*
darxend_ftp_dir_list_get_last(DarxendFtpDirList* self)
{
	USING_PRIVATE(self)

	GList* pentries = g_list_last(priv->entries);
	while (pentries)
	{
		DarxendFtpEntry* entry = (DarxendFtpEntry*)pentries->data;
		if (entry && !strcmp(entry->chrFileName, "sn.last"))
			 return entry;
		pentries = pentries->prev;
	}
	return NULL;
}

const DarxendFtpEntry*
darxend_ftp_dir_list_get_last_index(DarxendFtpDirList* self)
{
	USING_PRIVATE(self)

	GList* pentries = priv->entries;
	DarxendFtpEntry* previous = NULL;
	DarxendFtpEntry* current = NULL;
	while (pentries)
	{
		DarxendFtpEntry* entry = (DarxendFtpEntry*)pentries->data;

		if (entry && strcmp(entry->chrFileName, "sn.last"))
		{
			previous = current;
			current = entry;
			if (isDateBGreater(previous, current))
				return previous;
		}
		else if (entry)
			return current;
		pentries = pentries->next;
	}
	return NULL;
}

DarxendFtpEntry**
darxend_ftp_dir_list_get_latest_entries(DarxendFtpDirList* self, int count)
{
	USING_PRIVATE(self)

	GList* pentries = priv->entries;

	DarxendFtpEntry** result = (DarxendFtpEntry**)calloc(count, sizeof(DarxendFtpEntry*));

	int i;
	for (i = 0; i < count; i++)
	{
		DarxendFtpEntry* entry = (DarxendFtpEntry*)pentries->data;
		DarxendFtpEntry* item = (DarxendFtpEntry*)malloc(sizeof(DarxendFtpEntry));
		item->chrFileName = strdup(entry->chrFileName);
		item->month = entry->month;
		item->day = entry->day;
		item->hour = entry->hour;
		item->min = entry->min;
		result[i] = item;

		pentries = g_list_next(pentries);

		if (pentries == NULL)
			return NULL;
	}

	return result;
}


/*********************
 * Private Functions *
 *********************/

static gboolean
isDateBGreater(DarxendFtpEntry* a, DarxendFtpEntry* b)
{
	return ((b->month > a->month) ||
			(b->month == a->month &&
				b->day > a->day) ||
			(b->month == a->month &&
				b->day == a->day &&
				b->hour > a->hour) ||
			(b->month == a->month &&
				b->day == a->day &&
				b->hour == a->hour &&
				b->min > a->min));
}

static gint
dir_sorter(gconstpointer a, gconstpointer b)
{
	DarxendFtpEntry* aa = (DarxendFtpEntry*)a;
	DarxendFtpEntry* bb = (DarxendFtpEntry*)b;
	if (!strcmp(aa->chrFileName, "sn.last"))
		return 2;
	if (!strcmp(bb->chrFileName, "sn.last"))
		return -2;
	if (isDateBGreater(aa, bb))
		return 1;
	return -1;
}

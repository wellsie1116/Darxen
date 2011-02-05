/* FtpDirList.h
 *
 * Copyright (C) 2009 - Kevin Wells <kevin@darxen.org>
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

#ifndef FTPDIRLIST_H_
#define FTPDIRLIST_H_

#include <glib-object.h>

#include <glib/gtypes.h>
#include <glib/glist.h>

G_BEGIN_DECLS

#define DARXEND_FTP_DIR_LIST_ERROR darxend_ftp_dir_list_error_quark()

#define DARXEND_TYPE_FTP_DIR_LIST				(darxend_ftp_dir_list_get_type())
#define DARXEND_FTP_DIR_LIST(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEND_TYPE_FTP_DIR_LIST, DarxendFtpDirList))
#define DARXEND_FTP_DIR_LIST_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEND_TYPE_FTP_DIR_LIST, DarxendFtpDirListClass))
#define DARXEND_IS_FTP_DIR_LIST(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEND_TYPE_FTP_DIR_LIST))
#define DARXEND_IS_FTP_DIR_LIST_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEND_TYPE_FTP_DIR_LIST))
#define DARXEND_FTP_DIR_LIST_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEND_TYPE_FTP_DIR_LIST, DarxendFtpDirListClass))

typedef struct _DarxendFtpDirList			DarxendFtpDirList;
typedef struct _DarxendFtpDirListClass		DarxendFtpDirListClass;
typedef struct _DarxendFtpEntry				DarxendFtpEntry;

struct _DarxendFtpDirList
{
	GObject parent;
};

struct _DarxendFtpDirListClass
{
	GObjectClass parent_class;

	/* signals */
	/* virtual funcs */
};

struct _DarxendFtpEntry
{
	char *chrFileName;
	int month;
	int day;
	int hour;
	int min;
};

typedef enum
{
	DARXEND_FTP_DIR_LIST_ERROR_FAILED
} DarxendFtpDirListError;

GType					darxend_ftp_dir_list_get_type	() G_GNUC_CONST;
DarxendFtpDirList*		darxend_ftp_dir_list_new		(char* dirlist);

const DarxendFtpEntry* 	darxend_ftp_dir_list_get_last(DarxendFtpDirList* self);
const DarxendFtpEntry* 	darxend_ftp_dir_list_get_last_index(DarxendFtpDirList* self);
DarxendFtpEntry** 		darxend_ftp_dir_list_get_latest_entries(DarxendFtpDirList* self, int count);

GQuark					darxend_ftp_dir_list_error_quark	();

G_END_DECLS

#endif /*FTPDIRLIST_H_*/

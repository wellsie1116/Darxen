/* FtpConnection.h
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

#ifndef FTPCONNECTION_H_
#define FTPCONNECTION_H_

#include <curl/curl.h>
#include "FtpDirList.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define DARXEN_FTP_CONNECTION_ERROR darxen_ftp_connection_error_quark()

#define DARXEN_TYPE_FTP_CONNECTION				(darxen_ftp_connection_get_type())
#define DARXEN_FTP_CONNECTION(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_FTP_CONNECTION, DarxenFtpConnection))
#define DARXEN_FTP_CONNECTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_FTP_CONNECTION, DarxenFtpConnectionClass))
#define DARXEN_IS_FTP_CONNECTION(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_FTP_CONNECTION))
#define DARXEN_IS_FTP_CONNECTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_FTP_CONNECTION))
#define DARXEN_FTP_CONNECTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_FTP_CONNECTION, DarxenFtpConnectionClass))

typedef struct _DarxenFtpConnection			DarxenFtpConnection;
typedef struct _DarxenFtpConnectionClass		DarxenFtpConnectionClass;

struct _DarxenFtpConnection
{
	GObject parent;
};

struct _DarxenFtpConnectionClass
{
	GObjectClass parent_class;

	/* signals */
	/* virtual funcs */
};

typedef enum
{
	DARXEN_FTP_CONNECTION_ERROR_FAILED
} DarxenFtpConnectionError;

GType					darxen_ftp_connection_get_type		() G_GNUC_CONST;
DarxenFtpConnection*	darxen_ftp_connection_new			();

void					darxen_ftp_connection_set_site(DarxenFtpConnection* self, char* site);
gboolean				darxen_ftp_connection_set_product(DarxenFtpConnection* self, char* product);

DarxendFtpEntry**		darxen_ftp_connection_get_latest_entries(DarxenFtpConnection* self);
char*					darxen_ftp_connection_read_entry(DarxenFtpConnection* self, DarxendFtpEntry* entry, int* size);

GQuark					darxen_ftp_connection_error_quark	();

G_END_DECLS

#endif /*FTPCONNECTION_H_*/

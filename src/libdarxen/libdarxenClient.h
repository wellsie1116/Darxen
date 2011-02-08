/*
 * libdarxenClient.h
 *
 *  Created on: Mar 23, 2010
 *      Author: wellska1
 */

#ifndef LIBDARXENCLIENT_H_
#define LIBDARXENCLIENT_H_

#include "libdarxenCommon.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define DARXEN_CLIENT_ERROR darxen_client_error_quark()

#define DARXEN_TYPE_CLIENT				(darxen_client_get_type())
#define DARXEN_CLIENT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), DARXEN_TYPE_CLIENT, DarxenClient))
#define DARXEN_CLIENT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), DARXEN_TYPE_CLIENT, DarxenClientClass))
#define DARXEN_IS_CLIENT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), DARXEN_TYPE_CLIENT))
#define DARXEN_IS_CLIENT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), DARXEN_TYPE_CLIENT))
#define DARXEN_CLIENT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), DARXEN_TYPE_CLIENT, DarxenClientClass))

typedef struct _DarxenClient			DarxenClient;
typedef struct _DarxenClientClass		DarxenClientClass;
typedef struct _DarxenClientDataPacket	DarxenClientDataPacket;

struct _DarxenClient
{
	GObject parent;
};

struct _DarxenClientDataPacket
{
	char* site;
	char* product;
	GDate* date;
	int time;
	char* data;
};

struct _DarxenClientClass
{
	GObjectClass parent_class;

	void (*data_received) (DarxenClient* client, DarxenClientDataPacket* data);
};

typedef enum
{
	DARXEN_CLIENT_ERROR_FAILED
} DarxenClientError;

GType			darxen_client_get_type	() G_GNUC_CONST;
DarxenClient*	darxen_client_new		(char* address, gboolean autostart);

gboolean		darxen_client_connect			(DarxenClient* self, GError** error);

/** Requests a certain number of the most recent frames from darxend without activating a poller for the site.
 *
 * If the requested number of frames exist in darxend's cache, they will be immediately returned.
 * Otherwise, a temporary poller will be added with a higer priority that will request count frames into the past.
 */
gboolean 		darxen_client_request_frames	(DarxenClient* self, const char* site, const char* product, int count, GError** error);

gboolean 		darxen_client_register_poller	(DarxenClient* self, const char* site, const char* product, GError** error);
gboolean 		darxen_client_unregister_poller	(DarxenClient* self, const char* site, const char* product, GError** error);




GQuark			darxen_client_error_quark	();

G_END_DECLS

#endif /* LIBDARXENCLIENT_H_ */

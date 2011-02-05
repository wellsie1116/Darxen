/*
 * libdarxenClient.c
 *
 *  Created on: Mar 23, 2010
 *      Author: wellska1
 */

#include "libdarxenClient.h"

#include "../soap/client/DarxenService.nsmap"

#include <time.h>
#include <pthread.h>
#include <unistd.h>

G_DEFINE_TYPE(DarxenClient, darxen_client, G_TYPE_OBJECT)

#define USING_PRIVATE(obj) DarxenClientPrivate* priv = DARXEN_CLIENT_GET_PRIVATE(obj);
#define DARXEN_CLIENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DARXEN_TYPE_CLIENT, DarxenClientPrivate))

static gchar** darxendArgs = NULL;

static void 	darxen_client_finalize(GObject* gobject);

static void*	polling_thread_run					(void* self);
static gboolean	polling_thread_notify_data_ready	(gpointer data);
static void		free_notify_data					(gpointer data);

enum
{
	SIGNAL_DATA_RECEIVED,

	LAST_SIGNAL
};

typedef struct _DarxenClientPrivate		DarxenClientPrivate;
typedef struct _ThreadNotifyData		ThreadNotifyData;
struct _DarxenClientPrivate
{
	struct soap* soap;

	char* address; // "http://127.0.0.1:4888/DarxenService"
	gboolean autostart;
	GPid darxendPid;

	int sessionID;

	gboolean pollerThreadUp;
	pthread_t pollerThread;
};

struct _ThreadNotifyData
{
	DarxenClient* client;
	DarxenClientDataPacket* packet;
};

static guint signals[LAST_SIGNAL] = {0,};

static void
darxen_client_class_init(DarxenClientClass *klass)
{
	g_type_class_add_private(klass, sizeof(DarxenClientPrivate));

	klass->parent_class.finalize = darxen_client_finalize;

	signals[SIGNAL_DATA_RECEIVED] = g_signal_new(
			"data-received",
			G_TYPE_FROM_CLASS(G_OBJECT_CLASS(klass)),
			G_SIGNAL_RUN_LAST,
			G_STRUCT_OFFSET(DarxenClientClass, data_received),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1, G_TYPE_POINTER);
}

static void
darxen_client_init(DarxenClient *client)
{
	USING_PRIVATE(client)

	priv->soap = NULL;

	priv->address = NULL;
	priv->autostart = 0;
	priv->darxendPid = NULL;

	priv->sessionID = 0;

	priv->pollerThreadUp = FALSE;
}

static void
darxen_client_finalize(GObject* gobject)
{
	USING_PRIVATE(DARXEN_CLIENT(gobject))

	g_debug("finalizing client");fflush(stdout);

	if (priv->sessionID)
	{
		if (soap_send_ns__goodbye_server(priv->soap, priv->address, NULL, priv->sessionID) != SOAP_OK)
		{
			g_warning("Failed to say disconnect from server safely");
		}
	}

	if (priv->pollerThreadUp)
		pthread_cancel(priv->pollerThread);

	if (priv->darxendPid)
	{
		g_spawn_close_pid(priv->darxendPid);
	}

	soap_free(priv->soap);

	/* Chain up to the parent class */
	G_OBJECT_CLASS(darxen_client_parent_class)->finalize(gobject);
}

/********************
 * Public Functions *
 ********************/

DarxenClient*
darxen_client_new(char* address, gboolean autostart)
{
	GObject *gobject = g_object_new(DARXEN_TYPE_CLIENT, NULL);
	DarxenClient *client = DARXEN_CLIENT(gobject);

	USING_PRIVATE(client)

	if (address)
		priv->address = strdup(address);

	priv->autostart = autostart;

	priv->soap = soap_new();
	priv->soap->send_timeout = 2; //FIXME: these values should probably be increased, when handling more data (these values are just for testing)
	priv->soap->recv_timeout = 2;

	return (DarxenClient*)gobject;
}

gboolean
darxen_client_connect(DarxenClient* self, GError** error)
{
	USING_PRIVATE(self)

	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	if (soap_call_ns__hello_server(priv->soap, priv->address, NULL, &priv->sessionID) != SOAP_OK)
	{
		g_debug("Failed to connect to web service");
		if (priv->autostart)
		{
			g_debug("Starting local darxend process");
			if (priv->address)
				free(priv->address);

			priv->address = strdup("http://127.0.0.1:4888/DarxenService");

			if (!darxendArgs)
			{
				darxendArgs = g_new(gchar*, 2);
				darxendArgs[0] = g_strdup("../darxend/Debug/darxend");
				darxendArgs[1] = NULL;
			}
			//fdopen may be useful when using pipes
			if (!g_spawn_async(NULL, darxendArgs, NULL, 0, NULL, NULL, &priv->darxendPid, error))
			{
				g_assert(error == NULL || *error != NULL);
				g_prefix_error(error, "Failed to start local server: ");
				return FALSE;
			}

			if (soap_call_ns__hello_server(priv->soap, priv->address, NULL, &priv->sessionID) != SOAP_OK)
			{
				g_set_error(error, DARXEN_CLIENT_ERROR, DARXEN_CLIENT_ERROR_FAILED, "Failed to connect to autostarted server!");
				g_spawn_close_pid(priv->darxendPid);
				return FALSE;
			}
		}
		else
		{
			g_set_error(error, DARXEN_CLIENT_ERROR, DARXEN_CLIENT_ERROR_FAILED, "Could not connect to darxend, and autostart is disabled");
			return FALSE;
		}
	}

	if (pthread_create(&priv->pollerThread, NULL, polling_thread_run, self))
	{
		g_set_error(error, DARXEN_CLIENT_ERROR, DARXEN_CLIENT_ERROR_FAILED, "Unable to start data poller thread");
		return FALSE;
	}
	priv->pollerThreadUp = TRUE;

	return TRUE;
}

gboolean
darxen_client_request_frames(DarxenClient* self, const char* site, const char* product, int count, GError** error)
{
	g_set_error(error, DARXEN_CLIENT_ERROR, DARXEN_CLIENT_ERROR_FAILED, "Not implemented");
	return FALSE;
}

gboolean
darxen_client_register_poller(DarxenClient* self, const char* site, const char* product, GError** error)
{
	USING_PRIVATE(self)

	if (!priv->sessionID)
	{
		g_set_error(error, DARXEN_CLIENT_ERROR, DARXEN_CLIENT_ERROR_FAILED, "Attempted to act on a nonactive session");
		return FALSE;
	}
	enum Bool result;

	if (soap_call_ns__add_radar_poller(priv->soap, priv->address, NULL, priv->sessionID, site, product, &result) != SOAP_OK)
	{
		char fault[500];
		g_set_error(error, DARXEN_CLIENT_ERROR, DARXEN_CLIENT_ERROR_FAILED, "Failed to register poller: %s", soap_sprint_fault(priv->soap, fault, 50));
		return FALSE;
	}

	return TRUE;
}

gboolean
darxen_client_unregister_poller(DarxenClient* self, const char* site, const char* product, GError** error)
{
	USING_PRIVATE(self)

	if (!priv->sessionID)
	{
		g_set_error(error, DARXEN_CLIENT_ERROR, DARXEN_CLIENT_ERROR_FAILED, "Attempted to act on a nonactive session");
		return FALSE;
	}

	if (soap_call_ns__remove_radar_poller(priv->soap, priv->address, NULL, priv->sessionID, site, product, NULL) != SOAP_OK)
	{
		char fault[50];
		g_set_error(error, DARXEN_CLIENT_ERROR, DARXEN_CLIENT_ERROR_FAILED, "Failed to unregister poller: %s", soap_sprint_fault(priv->soap, fault, 50));
		return FALSE;
	}

	return TRUE;
}

GQuark
darxen_client_error_quark()
{
	return g_quark_from_static_string("darxen-client-error-quark");
}

/*********************
 * Private Functions *
 *********************/

static void
polling_thread_cleanup(void* self)
{
	soap_free((struct soap*)self);
}

static void*
polling_thread_run(void* self)
{
	USING_PRIVATE((GObject*)self)

	struct soap* mySoap = soap_new();

	pthread_cleanup_push(polling_thread_cleanup, mySoap);

	while (1)
	{
		int res = 0;
//		mySoap->send_timeout = 0; //0 means no timeout
//		mySoap->recv_timeout = 0;

		if (soap_call_ns__poller_data_get_queue_count(mySoap, priv->address, NULL, priv->sessionID, &res) != SOAP_OK)
		{
			g_warning("getQueueCount failed, possibly due to timeout");fflush(stdout);
		}

		if (res)
		{
			pthread_testcancel();
			g_debug("Downloading %i pieces of data", res);fflush(stdout);

			struct RadarDataInfoArray data;
			if (soap_call_ns__poller_data_read_queue(mySoap, priv->address, NULL, priv->sessionID, res, &data) != SOAP_OK)
			{
				g_critical("call to readQueue failed!");fflush(stdout);
				pthread_exit(NULL);
				return NULL;
			}

			int i;
			for (i = 0; i < data.__size; i++)
			{
				struct RadarData radarData;
				struct RadarDataInfo* resultInfo = &data.array[i];

				if (soap_call_ns__get_radar_data(mySoap, priv->address, NULL, priv->sessionID, resultInfo->site, resultInfo->product, resultInfo->date, &radarData) != SOAP_OK)
				{
					g_critical("call to getRadarData failed!");fflush(stdout);
					continue;//FIXME: don't lose the RadarDataInfo structure
				}

				DarxenClientDataPacket* dataPacket = g_new(DarxenClientDataPacket, 1);
				dataPacket->site = strdup(resultInfo->site);
				dataPacket->product = strdup(resultInfo->product);
				dataPacket->date = g_date_new_dmy(resultInfo->date.date.day, resultInfo->date.date.month, resultInfo->date.date.year);
				dataPacket->time = resultInfo->date.time.hour * 60 * 60 + resultInfo->date.time.minute * 60;
				dataPacket->data = strdup(radarData.data);

				ThreadNotifyData* notifyData = (ThreadNotifyData*)malloc(sizeof(ThreadNotifyData));

				notifyData->client = (DarxenClient*)self;
				notifyData->packet = dataPacket;

				g_debug("Everything went OK, notifying main thread");fflush(stdout);

				g_idle_add_full(G_PRIORITY_DEFAULT, polling_thread_notify_data_ready, notifyData, free_notify_data);
			}

			soap_destroy(priv->soap);
			soap_end(priv->soap);
		}
		else
		{
			//the server messed up, delay a bit before our next request
			struct timespec delay = {5, 0};
			nanosleep(&delay, NULL);
		}

	}

	pthread_cleanup_pop(0);
	priv->pollerThreadUp = FALSE;
	return NULL;
}

static gboolean
polling_thread_notify_data_ready(gpointer data)
{
	ThreadNotifyData* notifyData = (ThreadNotifyData*)data;

	g_debug("Emitting data received signal");fflush(stdout);
	g_signal_emit(notifyData->client, signals[SIGNAL_DATA_RECEIVED], 0, notifyData->packet);

	return FALSE;
}

static void
free_notify_data(gpointer data)
{
	ThreadNotifyData* notifyData = (ThreadNotifyData*)data;
	DarxenClientDataPacket* dataPacket = notifyData->packet;

	free(dataPacket->data);
	free(dataPacket->site);
	free(dataPacket->product);
	g_date_free(dataPacket->date);
	g_free(dataPacket);

	free(notifyData);
}

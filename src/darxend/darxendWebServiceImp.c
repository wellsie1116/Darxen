/* darxendWebServiceImp.cc
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

#include "../soap/server/DarxenService.nsmap"
#include "RadarDataManager.h"
#include "ClientManager.h"
#include "Settings.h"

#include <glib.h>

static gboolean verifyDate(DateTime* date);

typedef struct soap soap;

int
die_bad_client(soap* soap)
{
	g_debug("Throwing invalid session fault");
	return soap_receiver_fault(soap, "Invalid session ID", NULL);
}

int
ns__hello_server(soap* soap, int* result)
{
	DarxendClient* client = client_manager_create_client();
	*result = client->ID;
	g_debug("Client connected with id=%i", *result);
	return SOAP_OK;
}

int
ns__goodbye_server(soap* soap, int sessionID)
{
	client_manager_kill_client(sessionID);

	return SOAP_OK;
}

int
ns__add_radar_poller(soap* soap, int sessionID, char* site, char* product, xsd__boolean* result)
{
	DarxendClient* client = client_manager_get_client(sessionID);
	if (!client)
		return die_bad_client(soap);

	darxend_client_add_poller(client, site, product);

	*result = true_;
	return SOAP_OK;
}

int
ns__remove_radar_poller(soap* soap, int sessionID, char* site, char* product, xsd__boolean* result)
{
	DarxendClient* client = client_manager_get_client(sessionID);
	if (!client)
		return die_bad_client(soap);

	darxend_client_remove_poller(client, site, product);

	*result = true_;
	return SOAP_OK;
}

int
ns__poller_data_get_queue_count(soap* soap, int sessionID, int* result)
{
	DarxendClient* client = client_manager_get_client(sessionID);
	if (!client)
		return die_bad_client(soap);
	*result = darxend_client_wait_queue_length(client);
	if (!darxend_client_is_valid(client))
		return die_bad_client(soap);
	return SOAP_OK;
}

int
ns__poller_data_read_queue(soap* soap, int sessionID, int count, struct RadarDataInfoArray* result)
{
	DarxendClient* client = client_manager_get_client(sessionID);
	if (!client)
		return die_bad_client(soap);

	RadarDataInfo* infos = darxend_client_read_queue(client, count);
	if (infos)
	{
		result->__size = count;

		result->array = (RadarDataInfo*)soap_malloc(soap, sizeof(RadarDataInfo) * count);
		int i;
		for (i = 0; i < count; i++)
		{
			result->array[i] = infos[i];
			result->array[i].site = soap_strdup(soap, result->array[i].site);
			result->array[i].product = soap_strdup(soap, result->array[i].product);
			free(infos[i].site);
			free(infos[i].product);
		}
		free(infos);
	}
	else
	{
		result->__size = -1;
	}
	return SOAP_OK;
}

int
ns__get_radar_data(soap* soap, int sessionID, char* site, char* product, struct DateTime time, struct RadarData* result)
{
	DarxendClient* client = client_manager_get_client(sessionID);
	if (!client)
		return die_bad_client(soap);

	unsigned int length = 0;

	gchar* psite = g_ascii_strdown(site, -1);
	gchar* pproduct = g_ascii_strup(product, -1);

	char* data = radar_data_manager_read_data(psite, pproduct, time, &length);

	g_free(psite);
	g_free(pproduct);

	if (!data)
		return soap_receiver_fault(soap, "Data not found", NULL);

	gchar* b64data = g_base64_encode((guchar*)data, length);
	free(data);
	result->data = (char*)soap_malloc(soap, strlen(b64data));
	strcpy(result->data, b64data);
	g_free(b64data);

	return SOAP_OK;
}

int
ns__search_radar_data(	soap* soap, int sessionID, char* site, char* product,
						struct DateTime* start, struct DateTime* end,
						int* result)
{
	DarxendClient* client = client_manager_get_client(sessionID);
	if (!client)
		return die_bad_client(soap);

	if (!start)
		return soap_receiver_fault(soap, "Start date must not be null", NULL);
	if (!verifyDate(start))
			return soap_receiver_fault(soap, "Invalid start date", "Value out of range");
	if (end)
	{
		if (!verifyDate(end))
			return soap_receiver_fault(soap, "Invalid end date", "Value out of range");
		if ((start->date.year > end->date.year) ||
			(start->date.year == end->date.year &&
				start->date.month > end->date.month) ||
			(start->date.year == end->date.year &&
				start->date.month == end->date.month &&
				start->date.day > end->date.day) ||
			(start->date.year == end->date.year &&
				start->date.month == end->date.month &&
				start->date.day == end->date.day &&
				start->time.hour > end->time.hour) ||
			(start->date.year == end->date.year &&
				start->date.month == end->date.month &&
				start->date.day == end->date.day &&
				start->time.hour == end->time.hour &&
				start->time.minute > end->time.minute))
			return soap_receiver_fault(soap, "Start date must be before end date.", NULL);
	}

	gchar* psite = g_ascii_strdown(site, -1);
	gchar* pproduct = g_ascii_strup(product, -1);

	*result = darxend_client_search(client, psite, pproduct, start, end);

	g_free(psite);
	g_free(pproduct);

	return SOAP_OK;
}

int
ns__search_get_count(soap* soap, int sessionID, int searchID, int* result)
{
	DarxendClient* client = client_manager_get_client(sessionID);
	if (!client)
		return die_bad_client(soap);

	*result = radar_data_manager_get_search_size(searchID);
	return SOAP_OK;
}

int
ns__search_get_records(soap* soap, int sessionID, int searchID,
							int start, int count, struct DateTimeArray* result)
{
	DarxendClient* client = client_manager_get_client(sessionID);
	if (!client)
		return die_bad_client(soap);

	DateTime* results = radar_data_manager_get_search_records(searchID, start, count);

	result->__size = count;
	result->array = soap_malloc(soap, count * sizeof(DateTime));
	memcpy(result->array, results, count*sizeof(DateTime));
	free(results);

	if (!result->array)
		return soap_receiver_fault(soap, "Invalid search request", "No results");
	return SOAP_OK;
}

int
ns__search_free(soap* soap, int sessionID, int searchID, xsd__boolean* result)
{
	DarxendClient* client = client_manager_get_client(sessionID);
	if (!client)
		return die_bad_client(soap);

	if (radar_data_manager_free_search(searchID))
		*result = true_;
	else
		*result = false_;
	return SOAP_OK;
}

static gboolean
verifyDate(DateTime* date)
{
	return ((g_date_valid_dmy((GDateDay)date->date.day, (GDateMonth)date->date.month, (GDateYear)date->date.year)) &&
			(date->time.hour >= 0 && date->time.hour < 25) &&
			(date->time.minute >= 0 && date->time.minute < 61));
}

/* RadarDataManager.c
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

#include "RadarDataManager.h"
#include "Client.h"
#include "Settings.h"
#include "Callbacks.h"
#include "FtpConnection.h"

#include <time.h>
#include <string.h>

#define POLLING_DELAY (60 * 2)
//#define POLLING_DELAY (20000)

typedef struct _SitePollers			SitePollers;
typedef struct _SiteProductPollers	SiteProductPollers;

struct _SiteProductPollers
{
	GList* clients; //Client
	pthread_mutex_t lockClients;
};

struct _SitePollers
{
	GHashTable* products; //SiteProductPollers
	pthread_mutex_t lockProducts;
};

static GHashTable* sites; //SitePollers
static GTree* results; //GList //DateTime
static int intSearchCounter;
static gboolean pollQueued = FALSE;

static pthread_mutex_t lockSites;
static pthread_mutex_t lockSearches;

static gint treeIntComparer(gconstpointer a, gconstpointer b);
static gint listTimeComparer(gconstpointer a, gconstpointer b);

static GList* searchYears(const gchar* path, DateTime* start, DateTime* end);
static GList* searchMonths(int year, const gchar* path, DateTime* start, DateTime* end);
static GList* searchDays(int year, int month, const gchar* path, DateTime* start, DateTime* end);
static GList* searchDay(int year, int month, int day, const gchar* path, DateTime* start, DateTime* end);

static gboolean pollData(gpointer userData);
static gboolean pollDataOnce(gpointer userData);
static void pollSite(DarxenFtpConnection* ftp, char* site, char* product);

static void printPollers();

void
radar_data_manager_init()
{
	sites = g_hash_table_new_full(g_str_hash, g_str_equal, free, (GDestroyNotify)g_hash_table_destroy);
	results = g_tree_new(treeIntComparer);
	intSearchCounter = 1;
	pollQueued = FALSE;
	pthread_mutex_init(&lockSites, NULL);
	pthread_mutex_init(&lockSearches, NULL);

	g_timeout_add_seconds(POLLING_DELAY, pollData, NULL);
}

void
radar_data_manager_add_poller(DarxendClient* client, char* site, char* product)
{
	SitePollers* siteInfo = (SitePollers*)g_hash_table_lookup(sites, site);

	if (!siteInfo)
	{
		siteInfo = (SitePollers*)malloc(sizeof(SitePollers));
		siteInfo->products = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);
		pthread_mutex_init(&siteInfo->lockProducts, NULL);
		g_hash_table_insert(sites, strdup(site), siteInfo);
	}
	pthread_mutex_lock(&siteInfo->lockProducts);
	{

		SiteProductPollers* productInfo = (SiteProductPollers*)g_hash_table_lookup(siteInfo->products, product);
		if (!productInfo)
		{
			productInfo = (SiteProductPollers*)malloc(sizeof(SiteProductPollers));
			productInfo->clients = NULL;
			pthread_mutex_init(&productInfo->lockClients, NULL);
			g_hash_table_insert(siteInfo->products, strdup(product), productInfo);
		}
		pthread_mutex_lock(&productInfo->lockClients);
		{
			//FIXME: We should really validate that the product is valid...
			productInfo->clients = g_list_prepend(productInfo->clients, client);
		}
		pthread_mutex_unlock(&productInfo->lockClients);

		if (client && !pollQueued)
		{
			pollQueued = TRUE;
			g_timeout_add(0, pollDataOnce, (gpointer)time(0));
			//FIXME: we want to unblock the main loop, but it doesn't work
			g_main_context_wakeup(g_main_context_default());
		}
	}
	pthread_mutex_unlock(&siteInfo->lockProducts);
	printPollers();
}

void
radar_data_manager_remove_poller(DarxendClient* client, char* site, char* product)
{
	SitePollers* siteInfo = (SitePollers*)g_hash_table_lookup(sites, site);
	if (!siteInfo)
	{
		g_warning("Attempted to remove watch that doesn't exist (invalid site: %s)", site);
		return;
	}
	pthread_mutex_lock(&siteInfo->lockProducts);
	{
		SiteProductPollers* productInfo = (SiteProductPollers*)g_hash_table_lookup(siteInfo->products, product);
		if (!productInfo || !productInfo->clients)
		{
			g_warning("Attempted to remove watch that doesn't exist (invalid product: %s)", product);
			pthread_mutex_unlock(&siteInfo->lockProducts);
			return;
		}
		pthread_mutex_lock(&productInfo->lockClients);
		productInfo->clients = g_list_remove(productInfo->clients, client);
		if (!productInfo->clients)
		{
			g_hash_table_remove(siteInfo->products, product);
			pthread_mutex_unlock(&productInfo->lockClients);
			pthread_mutex_destroy(&productInfo->lockClients);
			free(productInfo);
		}
		else
		{
			pthread_mutex_unlock(&productInfo->lockClients);
		}
	}
	pthread_mutex_unlock(&siteInfo->lockProducts);
	printPollers();
}

void
radar_data_manager_iter_pollers(DarxendClient* client, PollerIterFunc callback, void* data)
{
	pthread_mutex_lock(&lockSites);
	if (!sites)
		return;
	GList* sitesKeys = g_hash_table_get_keys(sites);
	GList* psitesKeys = sitesKeys;
	while (psitesKeys)
	{
		char* site = (char*)psitesKeys->data;
		SitePollers* siteInfo = (SitePollers*)g_hash_table_lookup(sites, site);
		pthread_mutex_lock(&siteInfo->lockProducts);
		GList* products = g_hash_table_get_keys(siteInfo->products);
		GList* pproducts = products;
		while (pproducts)
		{
			char* product = (char*)pproducts->data;
			SiteProductPollers* productInfo = (SiteProductPollers*)g_hash_table_lookup(siteInfo->products, product);
			pthread_mutex_lock(&productInfo->lockClients);
			{
				GList* clients = productInfo->clients;
				while (clients)
				{
					DarxendClient* other = (DarxendClient*)clients->data;
					if (other && other->ID == client->ID)
					{
						callback(site, product, data);
						break;
					}
					clients = clients->next;
				}
			}
			pthread_mutex_unlock(&productInfo->lockClients);
			pproducts = pproducts->next;
		}
		g_list_free(products);
		pthread_mutex_unlock(&siteInfo->lockProducts);

		psitesKeys = psitesKeys->next;
	}
	g_list_free(sitesKeys);
	pthread_mutex_unlock(&lockSites);
}

int
radar_data_manager_search_past_data(char* site, char* product, int frames)
{
	//TODO: Implement
//	time_t rawtime = time(NULL);
//	tm* utc = gmtime(&rawtime);
//	end = new DateTime();
//	end->date.year = 1900 + utc->tm_year;
//	end->date.month = utc->tm_mon + 1;
//	end->date.day = utc->tm_mday;
//	end->time.hour = utc->tm_hour;
//	end->time.minute = utc->tm_min;



	return 0;
}

int
radar_data_manager_search(char* site, char* product, DateTime* start, DateTime* end)
{
	int searchID = intSearchCounter;
	gchar* path = g_build_filename("archives", "level3", site, product, NULL);
	if (!settings_path_exists(path))
	{
		g_message("The product path does not exist: %s\n", path);
		g_free(path);
		return 0;
	}
	GList* resultList;
	if (!end)
	{
		time_t rawtime = time(NULL);
		struct tm* utc = gmtime(&rawtime);
		end = g_new(DateTime, 1);
		end->date.year = 1900 + utc->tm_year;
		end->date.month = utc->tm_mon + 1;
		end->date.day = utc->tm_mday;
		end->time.hour = utc->tm_hour;
		end->time.minute = utc->tm_min;
		resultList = searchYears(path, start, end);
		g_free(end);
		end = NULL;
	}
	else
	{
		resultList = searchYears(path, start, end);
	}

	if (!resultList)
	{
		g_free(path);
		return 0;
	}
	pthread_mutex_lock(&lockSearches);
	g_tree_insert(results, GINT_TO_POINTER(searchID), resultList);
	intSearchCounter++;
	pthread_mutex_unlock(&lockSearches);

	free(path);

	return searchID;
}

int
radar_data_manager_get_search_size(int searchID)
{
	pthread_mutex_lock(&lockSearches);
	GSList* data = (GSList*)g_tree_lookup(results, GINT_TO_POINTER(searchID));
	if (!data)
	{
		pthread_mutex_unlock(&lockSearches);
		return -1;
	}
	int len = g_slist_length(data);
	pthread_mutex_unlock(&lockSearches);
	return len;
}

DateTime*
radar_data_manager_get_search_records(int searchID, int start, int count)
{
	pthread_mutex_lock(&lockSearches);
	GSList* data = (GSList*)g_tree_lookup(results, GINT_TO_POINTER(searchID));
	if (!data || count < 1 || (guint)(start + count) > g_slist_length(data))
	{
		pthread_mutex_unlock(&lockSearches);
		return NULL;
	}
	DateTime* result = (DateTime*)calloc(count, sizeof(DateTime));
	data = g_slist_nth(data, start);
	int i;
	for (i = 0; i < count; i++)
	{
		result[i] = *(DateTime*)data->data;
		data = data->next;
	}
	pthread_mutex_unlock(&lockSearches);
	return result;
}

gboolean
radar_data_manager_free_search(int searchID)
{
	pthread_mutex_lock(&lockSearches);
	GSList* data = (GSList*)g_tree_lookup(results, GINT_TO_POINTER(searchID));
	if (!data)
	{
		pthread_mutex_unlock(&lockSearches);
		return FALSE;
	}
	g_slist_free(data);
	g_tree_remove(results, &searchID);
	pthread_mutex_unlock(&lockSearches);
	return TRUE;
}

FILE*
radar_data_manager_read_data_file(char* site, char* product, DateTime date)
{
	char strYear[5];
	char strMonth[3];
	char strDay[3];
	char strTime[5];
	sprintf(strYear, "%i", date.date.year);
	sprintf(strMonth, "%i", date.date.month);
	sprintf(strDay, "%i", date.date.day);
	sprintf(strTime, "%02d%02d", date.time.hour, date.time.minute);
	gchar* path = g_build_filename("archives", "level3", site, product, strYear, strMonth, strDay, strTime, NULL);
	if (!settings_file_exists(path))
	{
		g_free(path);
		return NULL;
	}
	gchar* fullpath = settings_get_path(path);
	g_free(path);
	FILE* fin = fopen(fullpath, "rb");
	g_free(fullpath);
	return fin;
}

char*
radar_data_manager_read_data(char* site, char* product, DateTime date, unsigned int* length)
{
	FILE* fin = radar_data_manager_read_data_file(site, product, date);

	if (!fin)
	{
		*length = 0;
		return NULL;
	}
	fseek(fin, 0, SEEK_END);
	*length = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	char* result = (char*)calloc(*length, sizeof(char));
	if (fread(result, 1, *length, fin) != *length)
	{
		free(result);
		fclose(fin);
		*length = 0;
		return NULL;
	}
	fclose(fin);
	return result;
}

/*********************
 * Private Functions *
 *********************/

static gint
treeIntComparer(gconstpointer a, gconstpointer b)
{
	int aa = GPOINTER_TO_INT(a);
	int bb = GPOINTER_TO_INT(b);
	if (aa == bb)
		return 0;
	if (aa < bb)
		return -1;
	return 1;
}

static gint
listTimeComparer(gconstpointer a, gconstpointer b)
{
	DateTime* aa = (DateTime*)a;
	DateTime* bb = (DateTime*)b;
	if (aa->time.hour == bb->time.hour && aa->time.minute == bb->time.minute)
		return 0;
	if (aa->time.hour < bb->time.hour ||
		(aa->time.hour == bb->time.hour && aa->time.minute < bb->time.minute))
		return -1;
	return 1;
}

static GList*
searchYears(const gchar* path, DateTime* start, DateTime* end)
{
	GList* results = NULL;
	GList* presults = NULL;
	int year;
	for (year = start->date.year; year <= end->date.year; year++)
	{
		char strYear[5];
		sprintf(strYear, "%i", year);
		gchar* yearPath = g_build_filename(path, strYear, NULL);
		if (settings_path_exists(yearPath))
		{
			GList* yearResults = searchMonths(year, yearPath, start, end);
			if (!presults)
				presults = yearResults;
			else
				presults->next = yearResults;
			if (!results)
				results = presults;
			presults = g_list_last(presults);
		}
		free(yearPath);
	}
	return results;
}

static GList*
searchMonths(int year, const gchar* path, DateTime* start, DateTime* end)
{
	int monthStart = (start->date.year == year) ? start->date.month : 1;
	int monthEnd = (end->date.year == year) ? end->date.month : 12;
	GList* results = NULL;
	GList* presults = NULL;
	int month;
	for (month = monthStart; month <= monthEnd; month++)
	{
		char strMonth[3];
		sprintf(strMonth, "%i", month);
		gchar* monthPath = g_build_filename(path, strMonth, NULL);
		if (settings_path_exists(monthPath))
		{
			GList* monthResults = searchDays(year, month, monthPath, start, end);
			if (!presults)
				presults = monthResults;
			else
				presults->next = monthResults;
			if (!results)
				results = presults;
			presults = g_list_last(presults);
		}
		free(monthPath);
	}
	return results;
}

static GList*
searchDays(int year, int month, const gchar* path, DateTime* start, DateTime* end)
{
	int dayStart = (start->date.year == year && start->date.month == month) ? start->date.day : 1;
	int dayEnd = (end->date.year == year && end->date.month == month) ? end->date.day : 31;
	GList* results = NULL;
	GList* presults = NULL;
	int day;
	for (day = dayStart; day <= dayEnd; day++)
	{
		char strDay[3];
		sprintf(strDay, "%i", day);
		gchar* dayPath = g_build_filename(path, strDay, NULL);
		if (settings_path_exists(dayPath))
		{
			GList* dayResults = searchDay(year, month, day, dayPath, start, end);
			if (!presults)
				presults = dayResults;
			else
				presults->next = dayResults;
			if (!results)
				results = presults;
			presults = g_list_last(presults);
		}
		free(dayPath);
	}
	return results;
}

static GList*
searchDay(int year, int month, int day, const gchar* path, DateTime* start, DateTime* end)
{
	gchar* abspath = settings_get_path(path);
	GDir* dir = g_dir_open(abspath, 0, NULL);
	if (!dir)
	{
		g_free(abspath);
		return NULL;
	}
	int startHour = 0;
	int startMinute = 0;
	int endHour = 24;
	int endMinute = 60;
	if (start->date.year == year && start->date.month == month && start->date.day == day)
	{
		startHour = start->time.hour;
		startMinute = start->time.minute;
	}
	if (end->date.year == year && end->date.month == month && end->date.day == day)
	{
		endHour = end->time.hour;
		endMinute = end->time.minute;
	}
	GList* results = NULL;
	const gchar* fileName;
	int hour;
	int minute;
	while ((fileName = g_dir_read_name(dir)))
	{
		gchar* fullPath = g_build_filename(path, fileName, NULL);
		if (settings_file_exists(fullPath))
		{
			if (strlen(fileName) == 4 && sscanf(fileName, "%02d%02d", &hour, &minute) == 2)
			{
				if ((hour > startHour && hour < endHour) ||
					(hour == startHour && minute >= startMinute) ||
					(hour == endHour && minute <= endMinute))
				{
					DateTime* entry = (DateTime*)malloc(sizeof(DateTime));
					entry->date.year = year;
					entry->date.month = month;
					entry->date.day = day;
					entry->time.hour = hour;
					entry->time.minute = minute;
					results = g_list_append(results, entry);
				}
			}
		}
		g_free(fullPath);
	}

	g_dir_close(dir);
	results = g_list_sort(results, listTimeComparer);
	g_free(abspath);
	return results;
}

static gboolean
pollData(gpointer userData)
{
	printf("Polling data\n");
	if (!sites)
		g_warning("Sites is null!");
	DarxenFtpConnection* ftp = darxen_ftp_connection_new();

	pthread_mutex_lock(&lockSites);
	//get the list of keys and duplicate their values,
	//because the hash table can be changed when we release the lock
	GList* siteList = g_hash_table_get_keys(sites);
	GList* psiteList = siteList;
	while (psiteList)
	{
		psiteList->data = strdup((char*)psiteList->data);
		psiteList = psiteList->next;
	}
	pthread_mutex_unlock(&lockSites);

	psiteList = siteList;
	while (psiteList)
	{
		char* site = (char*)psiteList->data;

		pthread_mutex_lock(&lockSites);
		SitePollers* siteInfo = (SitePollers*)g_hash_table_lookup(sites, site);
		pthread_mutex_unlock(&lockSites);
		if (siteInfo)
		{
			pthread_mutex_lock(&siteInfo->lockProducts);
			//get the list of keys and duplicate their values,
			//because the hash table can be changed when we release the lock
			GList* products = g_hash_table_get_keys(siteInfo->products);
			GList* pproducts = products;
			while (pproducts)
			{
				pproducts->data = strdup((char*)pproducts->data);
				pproducts = pproducts->next;
			}
			pthread_mutex_unlock(&siteInfo->lockProducts);

			pproducts = products;
			while (pproducts)
			{
				char* product = (char*)pproducts->data;
				pthread_mutex_lock(&siteInfo->lockProducts);
				SiteProductPollers* productInfo = (SiteProductPollers*)g_hash_table_lookup(siteInfo->products, product);
				pthread_mutex_unlock(&siteInfo->lockProducts);
				if (productInfo)
				{
					pollSite(ftp, site, product);
				}
				else
				{
					g_debug("Product (%s) of site (%s) has been removed during a poll cycle", product, site);
				}

				free(product);
				pproducts = pproducts->next;
			}
			g_list_free(products);
		}
		else
		{
			g_debug("Site (%s) has been removed during a poll cycle", site);
		}

		free(site);
		psiteList = psiteList->next;
	}
	g_list_free(siteList);

	g_object_unref(G_OBJECT(ftp));

	return TRUE;
}

static gboolean
pollDataOnce(gpointer userData)
{
	printf("It took %f seconds to poll data in response to a poller\n", difftime(time(0), (time_t)userData));


	pollData(userData);
	pollQueued = FALSE;
	return FALSE;
}

static void
pollSite(DarxenFtpConnection* ftp, char* site, char* product)
{
	printf("Polling %s from %s\n", product, site);

	darxen_ftp_connection_set_site(ftp, site);
	if (!darxen_ftp_connection_set_product(ftp, product))
	{
		g_warning("Invalid product name requested: %s\n", product);
		return;
	}

	DarxendFtpEntry** entries = darxen_ftp_connection_get_latest_entries(ftp);

	if (entries == NULL)
	{
		g_warning("aborting poll for %s from %s", product, site);
		return;
	}

	int prevMonth = 0;
	int year = settings_get_current_year();
	int i;
	for (i = 0; i < 5; i++)
	{
		DarxendFtpEntry* entry = entries[i];
		if (prevMonth == 0)
		{
			prevMonth = entry->month;
		}
		else
		{
			if (entry->month > prevMonth)
			{
				year--;
				prevMonth = entry->month;
			}
		}
		//printf("Data for %s (len %i): %s\n", entry->chrFileName, size, data);
		char strYear[5];
		char strMonth[3];
		char strDay[3];
		char strTime[5];
		sprintf(strYear, "%i", year);
		sprintf(strMonth, "%i", entry->month);
		sprintf(strDay, "%i", entry->day);
		sprintf(strTime, "%02i%02i", entry->hour, entry->min);

		gchar* path = g_build_filename("archives", "level3", site, product, strYear, strMonth, strDay, strTime, NULL);
		if (!settings_file_exists(path))
		{
			int size;
			char* data = darxen_ftp_connection_read_entry(ftp, entry, &size);
			if (size > 0)
			{
				gchar* dir = g_build_filename("archives", "level3", site, product, strYear, strMonth, strDay, NULL);
				settings_ensure_path(dir);
				g_free(dir);
				gchar* fullpath = settings_get_path(path);
				printf("Writing data to path: %s\n", fullpath);
				FILE* fout = fopen(fullpath, "wb");
				if (fout != NULL)
				{
					if ((int)fwrite(data, 1, size, fout) != size)
						g_warning("Error writing radar data file (incomplete write)");
					fclose(fout);
					//Inform the clients that data has arrived for them
					pthread_mutex_lock(&lockSites);
					SitePollers* siteInfo = (SitePollers*)g_hash_table_lookup(sites, site);
					pthread_mutex_unlock(&lockSites);
					if (siteInfo)
					{
						pthread_mutex_lock(&siteInfo->lockProducts);
						SiteProductPollers* productInfo = (SiteProductPollers*)g_hash_table_lookup(siteInfo->products, product);
						pthread_mutex_unlock(&siteInfo->lockProducts);

						if (productInfo)
						{
							pthread_mutex_lock(&productInfo->lockClients);
							GList* clients = productInfo->clients;
							while (clients)
							{
								DarxendClient* client = (DarxendClient*)clients->data;
								//FIXME: the timestamp should be taken from the file itself, NOT the ftp entry
								if (client != NULL)
									darxend_client_add_to_queue(client, site, product, year, entry->month, entry->day, entry->hour, entry->min);
								clients = clients->next;
							}
							pthread_mutex_unlock(&productInfo->lockClients);
						}
						else
						{
							g_debug("Product (%s) of site (%s) has been removed during a poll cycle (informing phase)", product, site);
						}
					}
					else
					{
						g_debug("Site (%s) has been removed during a poll cycle (informing phase)", site);
					}
				}
				else
				{
					g_warning("Error writing radar data file (failed to open)");
				}
				g_free(fullpath);
			}
			else
			{
				g_warning("Error reading radar data file");
			}
		}
		else
		{
			break;
		}
		g_free(path);
	}

	for (i = 0; i < 5; i++)
	{
		free(entries[i]->chrFileName);
		free(entries[i]);
	}
	free(entries);
}

static void
printPollers()
{
	pthread_mutex_lock(&lockSites);
	printf("Active Pollers:\n");
	if (!sites)
		g_warning("Sites is null!");
	GList* sitesKeys = g_hash_table_get_keys(sites);
	GList* psitesKeys = sitesKeys;
	while (psitesKeys)
	{
		char* site = (char*)psitesKeys->data;
		printf("\tSite: %s\n", site);
		SitePollers* siteInfo = (SitePollers*)g_hash_table_lookup(sites, site);
		pthread_mutex_lock(&siteInfo->lockProducts);
		GList* products = g_hash_table_get_keys(siteInfo->products);
		GList* pproducts = products;
		while (pproducts)
		{
			char* product = (char*)pproducts->data;
			printf("\t\tProduct: %s - ", product);
			SiteProductPollers* productInfo = (SiteProductPollers*)g_hash_table_lookup(siteInfo->products, product);
			pthread_mutex_lock(&productInfo->lockClients);
			{
				GList* clients = productInfo->clients;
				while (clients)
				{
					DarxendClient* client = (DarxendClient*)clients->data;
					if (client == NULL)
						printf("x ");
					else
						printf("%i ", client->ID);
					clients = clients->next;
				}
			}
			pthread_mutex_unlock(&productInfo->lockClients);
			printf("\n");
			pproducts = pproducts->next;
		}
		g_list_free(products);
		pthread_mutex_unlock(&siteInfo->lockProducts);

		psitesKeys = psitesKeys->next;
	}
	g_list_free(sitesKeys);
	pthread_mutex_unlock(&lockSites);
}

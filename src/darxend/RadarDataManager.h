/* RadarDataManager.h
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

#ifndef RADARDATAMANAGER_H_
#define RADARDATAMANAGER_H_

#include <glib.h>
#include "restapi.h"

#include "Client.h"
#include "FtpConnection.h"

typedef void(*PollerIterFunc)(char* site, char* product, void* data);

void		radar_data_manager_init					();

void 		radar_data_manager_add_poller			(DarxendClient* client, char* site, char* product);
void		radar_data_manager_remove_poller		(DarxendClient* client, char* site, char* product);
void		radar_data_manager_iter_pollers			(DarxendClient* client, PollerIterFunc callback, void* data);

int			radar_data_manager_search_past_data		(char* site, char* product, int frames);

gint*		radar_data_manager_get_search_range		(char* site, char* product, int year, int month, int day, int* count);
int			radar_data_manager_search				(char* site, char* product, DateTime* start, DateTime* end);
int			radar_data_manager_get_search_size		(int searchID);
DateTime*	radar_data_manager_get_search_records	(int searchID, int start, int count);
gboolean	radar_data_manager_free_search			(int searchID);

FILE*		radar_data_manager_read_data_file		(char* site, char* product, DateTime date);
char*		radar_data_manager_read_data			(char* site, char* product, DateTime date, unsigned int* length);


#endif /*RADARDATAMANAGER_H_*/

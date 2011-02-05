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
#include "soap.h"

#include "Client.h"
#include "FtpConnection.h"


void		radar_data_manager_init					();

void 		radar_data_manager_add_poller			(DarxendClient* client, char* site, char* product);
void		radar_data_manager_remove_poller		(DarxendClient* client, char* site, char* product);

int			radar_data_manager_search_past_data		(char* site, char* product, int frames);

int			radar_data_manager_search				(char* site, char* product, struct DateTime* start, struct DateTime* end);
int			radar_data_manager_get_search_size		(int searchID);
DateTime*	radar_data_manager_get_search_records	(int searchID, int start, int count);
gboolean	radar_data_manager_free_search			(int searchID);

char*		radar_data_manager_read_data			(char* site, char* product, DateTime date, unsigned int* length);


#endif /*RADARDATAMANAGER_H_*/

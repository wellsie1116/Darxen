/* ClientManager.h
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

#ifndef CLIENTMANAGER_H_
#define CLIENTMANAGER_H_

#include "Client.h"

#include <glib.h>

void			client_manager_init				();
DarxendClient*	client_manager_create_client	();
gboolean		client_manager_validate_client	(int ID);
int				client_manager_kill_client		(int ID);
DarxendClient*	client_manager_get_client		(int ID);

#endif /*CLIENTMANAGER_H_*/

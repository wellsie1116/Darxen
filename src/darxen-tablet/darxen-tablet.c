/* darxen-tablet.c
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

#include "darxen-tablet.h"
#include "darxenconfig.h"

#include <libdarxenRestfulClient.h>

#include <glib.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void cleanup_client()
{
	DarxenRestfulClient* client = darxen_config_get_client(darxen_config_get_instance());

	darxen_restful_client_disconnect(client, NULL);
}

int main(int argc, char *argv[])
{
	g_type_init();
	DarxenRestfulClient* client = darxen_restful_client_new();
	int id = darxen_restful_client_connect(client, NULL);
	if (!id)
	{
		//attempt to spawn darxend ourselves
		g_message("Darxend server not found, spawning one ourselves");
		if (vfork())
		{
			sleep(1);
		}
		else
		{
			execl("./darxend", "darxend", NULL);
			execl("../darxend/darxend", "darxend", NULL);
			fprintf(stderr, "Failed to spawn darxend process, aborting");
			_exit(1);
		}
		id = darxen_restful_client_connect(client, NULL);
		if (!id)
		{
			g_error("Failed to connect to darxend process");
		}
	}
	darxen_config_set_client(darxen_config_get_instance(), client);

	int res;
	res = initialize_gui(&argc, &argv);

	atexit(cleanup_client);

	run_gui();

	return 0;
}


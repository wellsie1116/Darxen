/* darxend_test.c
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

#include "TestingCommon.h"

#include <glib.h>
#include <glib-object.h>

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int
main (int argc, char* argv[]) {
	g_type_init();
	g_test_init(&argc, &argv, 0);

	libdarxen_init();
	testing_common_init();

	//spawn our darxend daemon
	pid_t pid = vfork();
	if (!pid)
	{
		execl("../darxend/darxend", "darxend", NULL);
		execl("./darxend", "darxend", NULL);
		_exit(1);
	}
	else
	{
		sleep(1);
	}

	int res = g_test_run();
	kill(pid, 15);
	return res;
}

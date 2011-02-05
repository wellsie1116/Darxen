/* restapi.c
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

#include "restapi.h"

#include <curl/curl.h> //FIXME: include something less
#include <stdarg.h>
#include <stdint.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PORT 4889
#define PAGE "<html><head><title>libmicrohttpd demo</title>"\
             "</head><body>libmicrohttpd demo</body></html>"

static struct MHD_Daemon* d;

static int handle_request(	void * cls,
							struct MHD_Connection * connection,
							const char * url,
							const char * method,
							const char * version,
							const char * upload_data,
							size_t * upload_data_size,
							void ** ptr)
{
  static int dummy;
  const char * page = cls;
  struct MHD_Response * response;
  int ret;

  if (0 != strcmp(method, "GET"))
    return MHD_NO; /* unexpected method */
  if (&dummy != *ptr) 
    {
      /* The first time only the headers are valid,
         do not respond in the first round... */
      *ptr = &dummy;
      return MHD_YES;
    }
  if (0 != *upload_data_size)
    return MHD_NO; /* upload data in a GET!? */
  *ptr = NULL; /* clear context pointer */
  response = MHD_create_response_from_data(strlen(page),
					   (void*) page,
					   MHD_NO,
					   MHD_NO);
  ret = MHD_queue_response(connection,
			   MHD_HTTP_OK,
			   response);
  MHD_destroy_response(response);
  return ret;
}

int restapi_init()
{
  d = MHD_start_daemon(	MHD_USE_THREAD_PER_CONNECTION,
						PORT,
						NULL,
						NULL,
						&handle_request,
						NULL, //PAGE,
						MHD_OPTION_END);
  return d == NULL;
}

int restapi_shutdown()
{
	MHD_stop_daemon(d);
}


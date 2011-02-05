/*
 * libdarxenFileSupport.h
 *
 *  Created on: Mar 14, 2010
 *      Author: wellska1
 */

#ifndef LIBDARXENFILESUPPORT_H_
#define LIBDARXENFILESUPPORT_H_

#include "libdarxenCommon.h"

#include <stdarg.h>

G_EXPORT const char*	darxen_file_support_get_app_path();
G_EXPORT char*			darxen_file_support_get_overridable_file_path(const char *filename);
G_EXPORT char*			darxen_file_support_find_path(const char *first, ...);

#endif /* LIBDARXENFILESUPPORT_H_ */

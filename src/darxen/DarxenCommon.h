/* common.h

   Copyright (C) 2008 Kevin Wells <kevin@darxen.org>

   This file is part of darxen.

   darxen is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   darxen is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with darxen.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <malloc.h>

#include <glib.h>

/* Constants */

#define DEBUG 1
#if DEBUG
#define VERBOSE_CONVERSIONS		0
#define VERBOSE_MAINFORM		1
#define VERBOSE_RADARVIEWER		1
#define VERBOSE_PARSER			0
#define VERBOSE_PARSERLEVEL3	0
#define VERBOSE_SETTINGS		0
#endif

/* Macros */

#define DEG_TO_RAD(deg) ((deg/180.0)*G_PI)
#define RAD_TO_DEG(rad) ((rad/G_PI)*180.0)

#define IS_WITHIN_RANGE(value, min, max)	(((value) >= (min)) && ((value) <= (max)))

#ifdef G_OS_WIN32
#define G_EXPORT __declspec(dllexport)
#else
#define G_EXPORT
#endif

/* Functions */
G_EXPORT void darxen_print_date(GDate *date);

#endif


/* Conversions.h

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

#ifndef __CONVERSIONS_H__
#define __CONVERSIONS_H__

#include "libdarxenCommon.h"

#include <glib.h>

#include <stdio.h>

G_BEGIN_DECLS

typedef struct {
	int year;
	int month;
	int day;
	int hour;
	int minute;
} DateTime;

G_EXPORT gboolean id_to_datetime(const gchar* id, DateTime* res);
G_EXPORT gchar* datetime_to_id(DateTime datetime);

G_EXPORT void conv_get_HMS(float coord, int *intHours, int *intMinutes, int *intSeconds);

G_EXPORT GDate *darxen_conversions_get_date_from_mod_julian(int julian_day);

G_EXPORT int darxen_conversions_time_get_hours(int seconds);
G_EXPORT int darxen_conversions_time_get_minutes(int seconds);
G_EXPORT int darxen_conversions_time_get_seconds(int seconds);

G_EXPORT const char *darxen_conversions_format_date_time(GDate *date, int seconds);

G_EXPORT gchar* darxen_hexdump(const gchar* data, size_t length);

G_END_DECLS

#endif

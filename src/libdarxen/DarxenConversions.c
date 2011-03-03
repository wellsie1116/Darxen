/* Conversions.c

   Copyright (C) 2011 Kevin Wells <kevin@darxen.org>

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

#include "DarxenConversions.h"

#include <glib/gmem.h>

#include <string.h>

gchar *chrDateTime;

void
conv_get_HMS(float coord, int *intHours, int *intMinutes, int *intSeconds)
{
	*intHours = (int)coord;
	coord -= (float)*intHours;
	if (coord < 0.0)
		coord = -coord;
	coord = coord * 60.0f;
	*intMinutes = (int)coord;
	coord -= *intMinutes;
	coord = coord * 60.0f;
	*intSeconds = (int)coord;
}

GDate*
darxen_conversions_get_date_from_mod_julian(int julian_day)
{
	return g_date_new_julian(julian_day + 719161);
}

int
darxen_conversions_time_get_hours(int seconds)
{
	return (int)((double)seconds / 3600);
}

int darxen_conversions_time_get_minutes(int seconds)
{
	return (int)((double)(seconds - darxen_conversions_time_get_hours(seconds) * 3600) / 60);
}

int
darxen_conversions_time_get_seconds(int seconds)
{
	return seconds - darxen_conversions_time_get_hours(seconds) * 3600 - darxen_conversions_time_get_minutes(seconds) * 60;
}

const char chrDays[7][4] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char chrMonths[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

const char*
darxen_conversions_format_date_time(GDate *date, int seconds)
{
	int intWeekday;
	int intMonth;

	intWeekday = g_date_get_weekday(date) - 1;
	intMonth = g_date_get_month(date) - 1;

	if (chrDateTime)
		g_free(chrDateTime);

	chrDateTime = g_new0(char, 26);

	sprintf(chrDateTime, "%s %02i:%02i:%02iZ %i-%s-%i",
		chrDays[intWeekday],
		darxen_conversions_time_get_hours(seconds),
		darxen_conversions_time_get_minutes(seconds),
		darxen_conversions_time_get_seconds(seconds),
		g_date_get_day(date),
		chrMonths[intMonth],
		g_date_get_year(date)
		);

	return chrDateTime;
}

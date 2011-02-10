/* restapi.h
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

#ifndef RESTAPI_DTS0FMGJ

#define RESTAPI_DTS0FMGJ

typedef struct {
	int year;
	int month;
	int day;
} Date ;

typedef struct {
	int hour;
	int minute;
} Time;

typedef struct {
	Date date;
	Time time;
} DateTime;

typedef struct {
	char* site;
	char* product;
	DateTime date;
} RadarDataInfo;

int restapi_init();
int restapi_shutdown();

#endif /* end of include guard: RESTAPI_DTS0FMGJ */

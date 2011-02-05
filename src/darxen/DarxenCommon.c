/* common.c
  
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

#include "DarxenCommon.h"

#include <stdio.h>

void
darxen_print_date(GDate *date)
{
	printf("%i/%i/%i", g_date_get_month(date), g_date_get_day(date), g_date_get_year(date));
}


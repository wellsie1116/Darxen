/* DataStructs.c

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

#include "libdarxenCommon.h"
#include "DarxenDataStructs.h"

static char chrSite[5] = {0,};
static char chrProductCode[4] = {0,};

char*
data_wmo_get_site(unsigned char *chrWmoHeader)
{
//	chrSite[0] = chrWmoHeader[7];
//	chrSite[1] = chrWmoHeader[8];
//	chrSite[2] = chrWmoHeader[9];
//	chrSite[3] = chrWmoHeader[10];
//	chrSite[4] = 0x00;

	chrSite[0] = 'K';
	chrSite[1] = chrWmoHeader[24];
	chrSite[2] = chrWmoHeader[25];
	chrSite[3] = chrWmoHeader[26];
	chrSite[4] = 0x00;
	return chrSite;
}

char*
data_wmo_get_product_code(unsigned char *chrWmoHeader)
{
	chrProductCode[0] = chrWmoHeader[21];
	chrProductCode[1] = chrWmoHeader[22];
	chrProductCode[2] = chrWmoHeader[23];
	chrProductCode[3] = 0x00;
	return chrProductCode;
}




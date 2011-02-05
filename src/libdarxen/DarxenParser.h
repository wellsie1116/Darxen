/* parser.h

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

#ifndef __PARSER_H__
#define __PARSER_H__

#include "libdarxenCommon.h"

#include <stdio.h>

G_BEGIN_DECLS

void parser_end(FILE *objFile);
short int parser_read_short(FILE *objFile);
int parser_read_int(FILE *objFile);
float parser_read_float(FILE *objFile);
unsigned char parser_read_byte(FILE *objFile);
void parser_read_bytes(FILE *objFile, unsigned char *chrBuffer, int intCount);
int parser_read_null_string(FILE *objFile, char *chrBuffer);

/** Determines if the current machine is little endian or big endian
  *
  * @returns true if little endian, false if big endian
  */
gboolean parser_get_endianness();

G_END_DECLS

#endif

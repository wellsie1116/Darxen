/* parserLvl3.h

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

#ifndef __PARSERLVL3_H__
#define __PARSERLVL3_H__

#include "libdarxenCommon.h"

#include "DarxenParser.h"
#include "DarxenDataStructs.h"
#include <stdio.h>

G_BEGIN_DECLS

/** Parse NWS level 3 data from a file
  *
  * @param objFile The file to parse
  */
G_EXPORT ProductsLevel3Data* parser_lvl3_parse_file(FILE *objFile);

/** Free a previously allocated data packet
  *
  * @param data The data packet to free
  */
G_EXPORT void parser_lvl3_free(ProductsLevel3Data* data);

G_END_DECLS

#endif

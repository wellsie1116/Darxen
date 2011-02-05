/* frmMain.h

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

#ifndef __FRMMAIN_H__
#define __FRMMAIN_H__

#include "DarxenCommon.h"
#include "DarxenSettings.h"
#include <DarxenDataStructs.h>
#include "RadarViewer.h"
#include <DarxenParserLevel3.h>
#include "ntbkProducts.h"

#include <gtk/gtk.h>
#include <glade/glade-xml.h>

G_BEGIN_DECLS

enum
{
  COLUMN_SITE_ID,
  COLUMN_STATE,
  COLUMN_CITY,
  NUM_COLUMNS
};

void darxen_main_new(GtkWindow* main, GtkBuilder* builder);

G_END_DECLS

#endif /* __FRMMAIN_H__ */


# Copyright (C) 20118 Kevin Wells <kevin@darxen.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


bin_PROGRAMS=darxen-tablet
darxen_tablet_SOURCES = darxen-tablet.c darxen-tablet.h \
						darxengui.c \
						darxenconfig.c darxenconfig.h \
						darxenaddviewdialog.c darxenaddviewdialog.h \
						darxenmainview.c darxenmainview.h \
						darxenradarviewer.c darxenradarviewer.h \
						darxenview.c darxenview.h \
						darxenviewconfig.c darxenviewconfig.h \
						darxenpanelmanager.c darxenpanelmanager.h \
						darxensitelist.c darxensitelist.h \
						../libdarxen/libdarxen.la gltk/libgltk.la

darxen_tablet_CFLAGS = -Wall -g -I../libdarxen $(json_CFLAGS) $(gtkglext_CFLAGS) $(libgrip_CFLAGS) $(curl_CFLAGS) $(libxml_CFLAGS) -O0
darxen_tablet_LDADD = $(json_LIBS) $(gtkglext_LIBS) $(libgrip_LIBS) $(curl_LIBS) $(libxml_LIBS) ../libdarxen/libdarxen.la gltk/libgltk.la -lpthread $(libshp_LIBS) -lshp # -lwsock32
darxen_tablet_LDFLAGS = -g

AM_LDFLAGS = -export-dynamic

# datadir=$(prefix)/@PACKAGE@/darxen-tablet

shapefiledir=$(datadir)/@PACKAGE@/darxen-tablet/shapefiles
shapefile_DATA= shapefiles/*

palettesdir=$(datadir)/@PACKAGE@/darxen-tablet/palettes
palettes_DATA= palettes/*

darxen_tablet_datadir=$(datadir)/@PACKAGE@/darxen-tablet
darxen_tablet_data_DATA=Shapefiles.xml Sites.xml UnBatang.ttf

EXTRA_DIST = shapefiles Shapefiles.xml Sites.xml UnBatang.ttf palettes

SUBDIRS = gltk

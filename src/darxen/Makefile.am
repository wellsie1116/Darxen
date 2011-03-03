# Copyright (C) 2008 Kevin Wells <kevin@darxen.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


bin_PROGRAMS=darxen
darxen_SOURCES = DarxenCommon.c DarxenCommon.h \
	DarxenConversions.c DarxenConversions.h \
	DarxenRenderer.c DarxenRenderer.h \
	DarxenRendererShared.c DarxenRendererShared.h \
	DarxenSettings.c DarxenSettings.h \
	frmMain.c frmMain.h \
	frmSplash.c frmSplash.h \
	ntbkProducts.c ntbkProducts.h \
	RadarViewer.c RadarViewer.h \
	darxen.c \
	../libdarxen/libdarxen.la

darxen_CFLAGS = -Wall -I../libdarxen $(gtkglext_CFLAGS) $(libglade_CFLAGS) $(libxml_CFLAGS) $(gsoap_CFLAGS)
darxen_LDADD = $(gtkglext_LIBS) $(libglade_LIBS) $(libxml_LIBS) $(gsoap_LIBS) ../libdarxen/libdarxen.la -lpthread $(libshp_LIBS) -lshp # -lwsock32
darxen_LDFLAGS = -g

AM_LDFLAGS = -export-dynamic

EXTRA_DIST = 

